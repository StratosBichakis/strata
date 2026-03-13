#!/usr/bin/env sh
BELA_IP=192.168.7.2
SYSROOT_DIR=$(pwd)/sysroot

# 1. Core Sync (Removed --safe-links to ensure symlinks are copied)
echo "--- Syncing System Libraries ---"
rsync -rvzLR --links \
      root@$BELA_IP:/usr/lib/arm-linux-gnueabihf \
      root@$BELA_IP:/usr/lib/gcc/arm-linux-gnueabihf \
      root@$BELA_IP:/usr/include \
      root@$BELA_IP:/lib/arm-linux-gnueabihf \
      "$SYSROOT_DIR/"

# 2. Bela-Specific Directories
mkdir -p "$SYSROOT_DIR/usr/xenomai/include" "$SYSROOT_DIR/usr/xenomai/lib"
mkdir -p "$SYSROOT_DIR/root/Bela/include" "$SYSROOT_DIR/root/Bela/lib"
mkdir -p "$SYSROOT_DIR/usr/local/include" "$SYSROOT_DIR/usr/local/lib"

# 3. Bela-Specific Syncs
rsync -avz root@$BELA_IP:/usr/xenomai/include/ "$SYSROOT_DIR/usr/xenomai/include"
rsync -avz root@$BELA_IP:/usr/xenomai/lib/ "$SYSROOT_DIR/usr/xenomai/lib"
rsync -avz root@$BELA_IP:/root/Bela/include/ "$SYSROOT_DIR/root/Bela/include"
rsync -avz root@$BELA_IP:/root/Bela/lib/ "$SYSROOT_DIR/root/Bela/lib"
rsync -avz root@$BELA_IP:/usr/local/include/ "$SYSROOT_DIR/usr/local/include"
rsync -avz root@$BELA_IP:/usr/local/lib/ "$SYSROOT_DIR/usr/local/lib"

# 4. Manual Perl Link
echo "--- Linking Perl ---"
ln -sif "$SYSROOT_DIR/usr/lib/arm-linux-gnueabihf/libperl.so.5.24" "$SYSROOT_DIR/usr/lib/arm-linux-gnueabihf/libperl.so"

# 5. Python Fixer for macOS Cross-Compilation
echo "--- Fixing Symlinks and Linker Scripts ---"
python3 - <<EOF
import os

sysroot = "$SYSROOT_DIR"
prefixes = ['/usr/lib/arm-linux-gnueabihf/', '/lib/arm-linux-gnueabihf/', '/usr/lib/', '/lib/']

for root, dirs, files in os.walk(sysroot):
    for name in files + dirs:
        path = os.path.join(root, name)
        # Fix absolute symlinks
        if os.path.islink(path):
            target = os.readlink(path)
            if target.startswith('/'):
                abs_target_path = os.path.join(sysroot, target.lstrip('/'))
                if os.path.exists(abs_target_path):
                    rel_target = os.path.relpath(abs_target_path, root)
                    os.remove(path)
                    os.symlink(rel_target, path)
        # Patch text-based linker scripts
        elif name.endswith('.so') and not os.path.islink(path):
            try:
                with open(path, 'rb') as f:
                    if f.read(4) == b'\x7fELF': continue
                with open(path, 'r', encoding='utf-8') as f:
                    content = f.read()
                if 'GROUP' in content or 'AS_NEEDED' in content:
                    new_content = content
                    for p in prefixes: new_content = new_content.replace(p, '')
                    with open(path, 'w', encoding='utf-8') as f:
                        f.write(new_content)
            except: continue
EOF
echo "--- Done ---"