#!/bin/bash
echo "=== Fixing LVGL Font Issue ==="
echo ""
echo "Step 1: Removing old sdkconfig..."
rm -f sdkconfig

echo "Step 2: Reconfiguring project (will use sdkconfig.defaults)..."
idf.py reconfigure

echo "Step 3: Building..."
idf.py build

echo ""
echo "Done! If successful, flash with: idf.py -p COM3 flash monitor"
