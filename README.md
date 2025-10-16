# C Blog Engine

[![Deploy](https://github.com/enoent0xFD/my_little_blog/actions/workflows/deploy.yml/badge.svg?branch=main)](https://github.com/enoent0xFD/my_little_blog/actions/workflows/deploy.yml)

A minimalist blog engine written in pure C, serving static files and markdown posts. Build from scratch with no frameworks - just core C and system calls.

## Features
- Static file serving
- Markdown post parsing with YAML frontmatter
- Real-time system stats
- Memory efficient (~2MB usage)
- Terminal-style UI
- IPv6 support
- Built-in security features

## Tech Stack
- Pure C
- md4c for markdown parsing
- cJSON for configuration
- No external web frameworks

## Development Setup
```bash
# Clone repository
git clone [your-repo-url]
cd c_blog_engine

# Build
make

# Run locally
./build/blog_server
```

## Project Structure
```
.
├── src/           # Source files
├── include/       # Header files
├── lib/           # External libraries
├── static/        # Static assets
├── content/       # Markdown blog posts
└── config.json    # Server configuration
```

## Configuration
```json
{
    "server": {
        "port": 8080,
        "static_dir": "./static",
        "blog_dir": "./content",
        "templates_dir": "./templates"
    }
}
```

## Writing Posts
Create markdown files in the `content` directory with YAML frontmatter:
```markdown
---
title: Your Post Title
date: 2024-12-31
preview: Brief description of your post
---

Content starts here...
```

## Deployment
See DEPLOY_HETZNER.md for a step-by-step guide to deploy on a Hetzner Cloud VM with HTTPS via Caddy. Docker and native systemd options are included.

## License
MIT
