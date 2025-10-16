FROM debian:bullseye-slim AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

# Copy only necessary files for building
WORKDIR /app
COPY Makefile ./
COPY src/ src/
COPY include/ include/
COPY lib/ lib/

# Build the application
RUN make

FROM debian:bullseye-slim
WORKDIR /app

# Copy only the necessary files
COPY --from=builder /app/build/blog_server .
COPY static/ static/
COPY content/ content/
COPY templates/ templates/
COPY config.json .

EXPOSE 8080

CMD ["./blog_server"]
