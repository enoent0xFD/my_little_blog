# C Blog Engine

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
        "blog_dir": "./content"
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
Configured for deployment on fly.io:
```bash
# Manual deployment
fly deploy

# Or use GitHub Actions
# Push to main branch
```

## License
MIT
