---
title: Pi Homelab Part 1 - Bootstrapping Kubernetes on Raspberry Pi
date: 2025-01-08
preview: Building a production-grade homelab from scratch using Ansible and k3s. From prepping the Pi to running a live Kubernetes cluster, this is the start of my infrastructure journey.
---

## My "Why": From Chaos to Kubernetes

After spending time on both sides of the development stack, I've realized my true calling lies in infrastructure. This project isn't just for fun; it's a core part of my journey, with my sights set on earning the CKA and CKAD certifications. To get there, I knew I had to go deep and really get my hands dirty.

Now, you might be asking, "Why infrastructure? Why choose the high-stress world of system outages, firefighting, and on-call alerts?" For me, the answer is simple: it’s where I feel at home. Having been diagnosed with ADHD, I've found that the intense focus required during a crisis is where I thrive. The chaos of a system outage doesn't overwhelm me; it brings a strange sense of clarity and purpose.

So, I figured it was time to get serious about building a homelab. The plan was to self-host some essential apps like Nextcloud, Pi-hole, and a nifty dashboard on Kubernetes. The catch? I was determined to do it by the book: everything automated, reproducible, and documented. Since I come from a coding background IaC seems like a natural choice.

**Hardware:** Single Raspberry Pi (planning to add more nodes later)  
**Software:** k3s (lightweight Kubernetes), Ansible (automation), Flux CD (GitOps)  
**Philosophy:** Infrastructure as Code. Everything in git.

---

## Project Structure

First things first—organizing the repo to keep things sane:

```
homelab/
├── ansible/
│   ├── inventory.yml          # Host definitions
│   └── playbooks/
│       ├── bootstrap.yml      # Prep the Pi for k3s
│       └── k3s-install.yml    # Install Kubernetes
├── kubernetes/
│   ├── infrastructure/        # Core cluster services
│   └── apps/                  # Application manifests
└── kubeconfig                 # Cluster access (gitignored)
```

This structure separates concerns: Ansible handles OS-level setup, Kubernetes manifests live under `kubernetes/`, and the kubeconfig stays local (never committed).

---

## The Setup: Hardware and My Dev Machine

Before we dive into the Ansible magic, let's talk about the gear I'm using.

For the homelab server, I'm starting with a single **Raspberry Pi 4 with 8GB of RAM**. It’s a beefy little machine and more than enough to get this project off the ground. Getting it ready was pretty painless. I used the official [Raspberry Pi Imager](https://github.com/raspberrypi/rpi-imager) to flash Ubuntu Server onto an SD card. The tool is great—it has a list of the most popular OS images right there, so you don't have to go hunting. During the setup, I configured it for SSH key authentication, and the rest was straightforward. (I did run into a few minor networking hiccups, but that's a story for another day.)

On the other side of the keyboard, I'm running a **Macbook Air M3**. It's a solid machine for most dev work, but I have a bad habit: over time, my laptop gets cluttered with dozens of packages and libraries from various projects. Eventually, things start to conflict, and I'm left wondering why some tool that worked yesterday is suddenly broken.

I decided it was time to end that chaos. My new rule: **isolated, reproducible environments for everything**. No more polluting my system with global dependencies. When I'm done with a project, I want to be able to just delete the folder, and *everything* related to it—all the tools, libraries, and dependencies—is gone for good.

For this, I'm using [**Devbox**](https://www.jetify.com/docs/devbox). It creates project-specific environments where every dependency is local first. It's been a game-changer for keeping my machine clean and my projects self-contained.

## Ansible Inventory

Before automating anything, I needed to tell Ansible where my Pi lives:

```yaml
all:
  hosts:
    rpi:
      ansible_host: 192.168.1.50
      ansible_user: filipm
      ansible_python_interpreter: /usr/bin/python3
      ansible_ssh_private_key_file: ~/.ssh/id_ed25519
  vars:
    k3s_version: v1.28.5+k3s1
    timezone: Europe/Belgrade
```

**Key decisions:**
- SSH key auth (no passwords)
- Pinned k3s version for reproducibility
- Variables at the inventory level for easy updates

---

## Bootstrap Playbook: Prepping the Pi

Kubernetes has specific requirements. The bootstrap playbook handles all the prerequisites:

### 1. Enable cgroups (required by k3s)

Raspberry Pi OS doesn't enable memory cgroups by default. We need to modify the kernel boot parameters:

```yaml
- name: Check if cgroup parameters exist
  shell: grep -q "cgroup_enable=cpuset" /boot/firmware/cmdline.txt
  register: cgroup_check
  failed_when: false
  changed_when: false

- name: Enable cgroup memory (firmware path)
  replace:
    path: /boot/firmware/cmdline.txt
    regexp: '^(.*rootwait)(.*)$'
    replace: '\1 cgroup_enable=cpuset cgroup_memory=1 cgroup_enable=memory\2'
  when: cgroup_check.rc != 0
```

**Why this approach?**
- Idempotent: checks before modifying, safe to rerun
- Handles both `/boot/firmware/cmdline.txt` (Ubuntu) and `/boot/cmdline.txt` (Raspberry Pi OS)
- Only reboots if changes were made

### 2. Disable swap

Kubernetes hates swap. Period. The kubelet will refuse to start if swap is enabled:

```yaml
- name: Disable swap
  command: swapoff -a
  when: ansible_swaptotal_mb > 0

- name: Remove swap from /etc/fstab
  replace:
    path: /etc/fstab
    regexp: '^([^#].*\s+swap\s+.*)$'
    replace: '# \1'
```

We comment out swap entries in `/etc/fstab` instead of deleting them—reversible changes are better.

### 3. Install dependencies

```yaml
- name: Install essential packages
  apt:
    name:
      - git
      - curl
      - nfs-common      # For NFS storage
      - open-iscsi      # For Longhorn/iSCSI storage
    state: present
    update_cache: yes
```

These packages enable network storage options down the road.

---

## Installing k3s

k3s is perfect for single-node clusters: small footprint, batteries included, production-ready.

### The installation playbook

```yaml
- name: Install k3s with embedded DB
  shell: |
    INSTALL_K3S_VERSION={{ k3s_version }} \
    INSTALL_K3S_EXEC="--disable traefik --disable servicelb --write-kubeconfig-mode 644" \
    /tmp/k3s-install.sh
  args:
    creates: /usr/local/bin/k3s
```

**Configuration choices:**
- `--disable traefik`: We'll use ingress-nginx instead (better for learning)
- `--disable servicelb`: We'll install MetalLB for proper LoadBalancer services
- `--write-kubeconfig-mode 644`: Makes kubeconfig readable without sudo

### Fetching the kubeconfig

The tricky part: getting cluster credentials from the Pi to my local machine.

```yaml
- name: Fetch kubeconfig to project directory
  fetch:
    src: /etc/rancher/k3s/k3s.yaml
    dest: "{{ playbook_dir }}/../../kubeconfig"
    flat: yes

- name: Fix kubeconfig server IP
  delegate_to: localhost
  become: no
  replace:
    path: "{{ playbook_dir }}/../../kubeconfig"
    regexp: 'https://127\.0\.0\.1:6443'
    replace: 'https://{{ ansible_host }}:6443'
```

**Why this matters:**
- k3s generates a kubeconfig with `server: https://127.0.0.1:6443`
- That won't work from my Mac—need the Pi's IP (192.168.1.50)
- `delegate_to: localhost` runs the replace task on my machine, not the Pi
- `become: no` prevents Ansible from trying to sudo on my Mac (which would fail)

---

## Tying It All Together: Accessing the Cluster

Alright, the `kubeconfig` is fetched and fixed. Now for the moment of truth! This is where the Devbox setup I mentioned earlier really shines. All my tools (`ansible`, `kubectl`, `flux`, etc.) are ready and waiting inside their own little sandbox, not cluttering up my Mac.

Let's fire up the shell and connect to our new Kubernetes cluster:

```bash
# Jump into the project's isolated environment
devbox shell

# Tell kubectl where to find our cluster credentials
export KUBECONFIG=$PWD/kubeconfig

# And... drumroll please... let's see our node!
kubectl get nodes
```

If everything went according to plan, you should see something beautiful like this:

```/dev/null/output.txt#L1-2
NAME      STATUS   ROLES                  AGE   VERSION
homelab   Ready    control-plane,master   11m   v1.28.5+k3s1
```

Success! We have a live, single-node Kubernetes cluster running on a Raspberry Pi, and we can talk to it from our dev machine.

---

## Lessons Learned

### 1. Idempotency is hard but worth it
Writing playbooks that can safely run multiple times requires thinking through every edge case. The cgroup check, swap detection, and file existence checks all prevent unintended side effects.

### 2. Path handling in Ansible is tricky
`./kubeconfig` doesn't work the way you'd expect—use `playbook_dir` variables or absolute paths.

### 3. `delegate_to: localhost` gotchas
Tasks delegated to localhost inherit the play's `become: yes` setting, causing sudo prompts on the wrong machine. Always add `become: no` for local tasks.

### 4. GitOps requires discipline
The kubeconfig file can't be committed (contains cluster certificates). Added to `.gitignore` immediately.

---

## What's Next

**Part 2** will cover:
- Installing MetalLB for LoadBalancer services
- Bootstrapping Flux CD for GitOps
- Deploying the first app (Pi-hole)

The cluster is ready. Now it's time to make it useful.

---

## Running the Playbooks

From the project root:

```bash
# Bootstrap the Pi
ansible-playbook -i ansible/inventory.yml ansible/playbooks/bootstrap.yml -K

# Install k3s
ansible-playbook -i ansible/inventory.yml ansible/playbooks/k3s-install.yml -K

# Configure kubectl
export KUBECONFIG=$PWD/kubeconfig
kubectl get nodes
```

Repository: [github.com/enoent0xFD/homelab](https://github.com/enoent0xFD/homelab)

---

*This is part 1 of the Pi Homelab series. Follow along as I build a production-grade Kubernetes homelab from scratch.*
