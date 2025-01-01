FROM debian:bullseye-slim AS builder
RUN apt-get update && apt-get install -y \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
RUN make

FROM debian:bullseye-slim
WORKDIR /app

COPY --from=builder /app/build/blog_server .
COPY static/ static/
COPY content/ content/
COPY config.json .

# Set proper permissions
RUN chmod +x ./blog_server && \
    chmod -R 755 static content

EXPOSE 8080

# Add healthcheck
HEALTHCHECK --interval=5s --timeout=3s \
    CMD curl -f http://localhost:8080/health || exit 1

CMD ["./blog_server"]
