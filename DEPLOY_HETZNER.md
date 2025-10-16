# Deploy to Hetzner with Docker + Caddy

This is an end-to-end onboarding guide for new team members to deploy the C blog server on Hetzner Cloud. It covers creating the VM, SSH setup, server hardening, Dockerized app, and HTTPS with Caddy.

## What You Need
- Hetzner Cloud account
- A domain you control (we’ll point an A record to the VM)
- A local machine with SSH and rsync

## 1) Create SSH Keys (local)
If you don’t already have one:
```bash
ssh-keygen -t ed25519 -C "your_email@example.com"
# default path: ~/.ssh/id_ed25519 (private) and ~/.ssh/id_ed25519.pub (public)
```

## 2) Create a Hetzner Project and Add Your SSH Key
- Log in to Hetzner Cloud Console
- Create a new Project (or reuse your team’s project)
- In Project → Security → SSH Keys → Add SSH Key
  - Name it after yourself
  - Paste the contents of your ~/.ssh/id_ed25519.pub

## 3) Create a Server (VM)
- Click “Add Server”
- Location: pick a region close to users (e.g., Falkenstein)
- Image: Ubuntu 22.04 LTS or 24.04 LTS
- Type: CPX11 (1 vCPU, 2GB RAM) is usually enough; scale as needed
- Networking: leave defaults (public IPv4)
- SSH keys: select the key you added in step 2
- Add a name (e.g., blog-prod-01) and create the server

Note the public IPv4 address; we’ll use it for DNS and SSH.

## 4) Point Your Domain to the VM
- In your domain DNS, create an A record for your site (e.g., `@` or `www`) pointing to the VM IP
- Wait for DNS to propagate (usually minutes; up to an hour)

## 5) First Login and Base System Setup
SSH into the server as root (or `ubuntu` if the image uses that user):
```bash
ssh root@<VM_IP>
# or
ssh ubuntu@<VM_IP>
```

Update packages and install basics:
```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y ufw rsync curl ca-certificates
```

Create a non-root user (if not provided by the image) and grant sudo:
```bash
sudo adduser deploy
sudo usermod -aG sudo deploy
sudo -u deploy mkdir -p /home/deploy/.ssh
sudo sh -c 'cat /root/.ssh/authorized_keys >> /home/deploy/.ssh/authorized_keys'
sudo chown -R deploy:deploy /home/deploy/.ssh
sudo chmod 700 /home/deploy/.ssh && sudo chmod 600 /home/deploy/.ssh/authorized_keys
```

Optionally disable password SSH (keys only):
```bash
sudo sed -i 's/^#\?PasswordAuthentication .*/PasswordAuthentication no/' /etc/ssh/sshd_config
sudo systemctl restart ssh
```

## 6) Configure Firewall (UFW)
Allow SSH, HTTP, and HTTPS; enable UFW:
```bash
sudo ufw allow OpenSSH
sudo ufw allow 80/tcp
sudo ufw allow 443/tcp
sudo ufw enable
```

## 7) Install Docker
```bash
curl -fsSL https://get.docker.com | sh
sudo usermod -aG docker $USER
# re-login for group to take effect or run with sudo
```

## 8) Install Caddy (HTTPS Reverse Proxy)
```bash
sudo apt install -y debian-keyring debian-archive-keyring apt-transport-https
curl -1sLf 'https://dl.cloudsmith.io/public/caddy/stable/gpg.key' | sudo tee /usr/share/keyrings/caddy-stable-archive-keyring.gpg >/dev/null
curl -1sLf 'https://dl.cloudsmith.io/public/caddy/stable/debian.deb.txt' | sudo tee /etc/apt/sources.list.d/caddy-stable.list
sudo apt update && sudo apt install -y caddy
```

Configure Caddy for your domain (replace `example.com`):
```bash
sudo bash -lc 'cat >/etc/caddy/Caddyfile <<EOF
example.com {
  reverse_proxy 127.0.0.1:8080
  encode zstd gzip
}
EOF'
sudo systemctl restart caddy
```

## 9) Upload the Project to the Server
From your local machine:
```bash
rsync -av --delete . deploy@<VM_IP>:/opt/blog/
```

## 10) Build and Run the Container
SSH to the server as your user (deploy):
```bash
ssh deploy@<VM_IP>
cd /opt/blog
docker build -t cblog:latest .
docker run -d --restart unless-stopped --name cblog -p 127.0.0.1:8080:8080 cblog:latest
```

## 11) Verify Deployment
- Visit: https://example.com
- Check Caddy: `sudo systemctl status caddy`
- App logs: `docker logs -f cblog`

## 12) Updating the App
On new commits, redeploy with:
```bash
rsync -av --delete . deploy@<VM_IP>:/opt/blog/
ssh deploy@<VM_IP> "cd /opt/blog && docker stop cblog && docker rm cblog && docker build -t cblog:latest . && docker run -d --restart unless-stopped --name cblog -p 127.0.0.1:8080:8080 cblog:latest"
```

## 13) Troubleshooting
- DNS not ready: wait for TTL or check with `dig A example.com`
- Caddy TLS fails: ensure port 80/443 open and domain resolves to this IP
- App 502 from Caddy: ensure the container is running and listening on 127.0.0.1:8080
- Permission errors with Docker: re-login after adding user to `docker` group or prefix commands with `sudo`

## Notes for Team
- App config lives in `config.json` (ports, dirs). The Docker image bundles `static/`, `content/`, `templates/`.
- The app listens on 8080 inside the container; Caddy proxies HTTPS traffic to it.
- Keep the app on localhost only (bound via `-p 127.0.0.1:8080:8080`).

## 14) Automated Deployments with GitHub Actions (CI/CD)
This section sets up automatic deploys to the Hetzner server on every push to `main` using GitHub Actions, GHCR (GitHub Container Registry), and SSH.

Prereqs (server):
- Docker is installed (see step 7)
- Your deploy user (e.g., `deploy`) belongs to the `docker` group (`sudo usermod -aG docker deploy`), then re-login
- Caddy is configured (step 8)

Secrets (GitHub repository → Settings → Secrets and variables → Actions):
- `HETZNER_HOST` – server IP or hostname
- `HETZNER_USER` – SSH user (e.g., `deploy`)
- `HETZNER_SSH_KEY` – private key for that user (contents of your `~/.ssh/id_ed25519`)
- Optional if GHCR package is private:
  - `GHCR_USERNAME` – your GitHub username
  - `GHCR_TOKEN` – a Personal Access Token with `read:packages` (server pull)

Notes on GHCR visibility:
- The workflow will push the image to GHCR using the built-in `GITHUB_TOKEN`.
- For the server to `docker pull`, either make the GHCR package public or provide `GHCR_USERNAME` and `GHCR_TOKEN` secrets so the server can log in to GHCR before pulling.

Add workflow file `.github/workflows/deploy.yml`:
```yaml
name: Deploy

on:
  push:
    branches: [ "main" ]

jobs:
  deploy:
    runs-on: ubuntu-latest
    permissions:
      contents: read
      packages: write

    steps:
      - name: Checkout
        uses: actions/checkout@v4

      - name: Set up Docker Buildx
        uses: docker/setup-buildx-action@v3

      - name: Login to GHCR
        uses: docker/login-action@v3
        with:
          registry: ghcr.io
          username: ${{ github.actor }}
          password: ${{ secrets.GITHUB_TOKEN }}

      - name: Prepare image name
        id: prep
        run: |
          IMAGE_ID=ghcr.io/${{ github.repository_owner }}/$(echo "${{ github.event.repository.name }}" | tr '[:upper:]' '[:lower:]')
          echo "IMAGE_ID=$IMAGE_ID" >> $GITHUB_ENV
          echo "TAG=${{ github.sha }}" >> $GITHUB_ENV

      - name: Build and push image
        uses: docker/build-push-action@v5
        with:
          context: .
          push: true
          tags: |
            ${{ env.IMAGE_ID }}:${{ env.TAG }}
            ${{ env.IMAGE_ID }}:latest

      - name: Deploy to Hetzner via SSH
        uses: appleboy/ssh-action@v1.0.3
        env:
          IMAGE_ID: ${{ env.IMAGE_ID }}
          TAG: ${{ env.TAG }}
          GHCR_USERNAME: ${{ secrets.GHCR_USERNAME }}
          GHCR_TOKEN: ${{ secrets.GHCR_TOKEN }}
        with:
          host: ${{ secrets.HETZNER_HOST }}
          username: ${{ secrets.HETZNER_USER }}
          key: ${{ secrets.HETZNER_SSH_KEY }}
          script: |
            set -e
            if [ -n "${GHCR_USERNAME}" ] && [ -n "${GHCR_TOKEN}" ]; then
              echo "${GHCR_TOKEN}" | docker login ghcr.io -u "${GHCR_USERNAME}" --password-stdin
            fi
            docker pull "$IMAGE_ID:latest" || docker pull "$IMAGE_ID:$TAG"
            docker stop cblog || true
            docker rm cblog || true
            docker run -d --restart unless-stopped --name cblog -p 127.0.0.1:8080:8080 "$IMAGE_ID:latest"
```

Triggering a deploy:
- Push to the `main` branch. Watch progress in the GitHub Actions tab.
- The job builds + pushes the GHCR image, then remotely pulls and restarts the container on the Hetzner server.

Common CI/CD issues:
- Permission denied for Docker on server: ensure `HETZNER_USER` is in `docker` group and re-login.
- GHCR pull denied: either make package public or set `GHCR_USERNAME` and `GHCR_TOKEN` secrets.
- Caddy 502: container not running or wrong port; check `docker ps` and logs.
