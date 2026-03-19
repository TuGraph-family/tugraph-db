#!/usr/bin/env bash
set -euo pipefail

# Install Cursor CLI inside the dev container (non-fatal if network fails)
curl -fsSL https://cursor.com/install | bash || true

# Ensure Cursor CLI is on PATH
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
export PATH="$HOME/.local/bin:$PATH"
