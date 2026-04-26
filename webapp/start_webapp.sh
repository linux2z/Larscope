#!/bin/bash
# Larscope WebApp Starter

echo "Installing WebApp dependencies..."
pip3 install flask

echo "Starting Larscope Virtual Diagnostics..."
python3 backend.py
