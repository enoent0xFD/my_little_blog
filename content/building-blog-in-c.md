---
title: Building a Blog Engine in C
date: 2024-12-31
preview: Exploring the fundamentals of web servers by implementing one from scratch using C. In this post, we'll dive deep into socket programming, HTTP protocols, and file serving.
---

This is my first blog post, written in Markdown and served by our C blog engine.

## Why Build a Blog Engine?

Because we can! And because it's a great way to understand how web servers work at a fundamental level.

## The Implementation

Here's some example code from our server:

```c
void handle_client(int client_fd) {
    // Handle incoming connections
    printf("Client connected!\n");
}
