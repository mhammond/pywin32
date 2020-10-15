#!/bin/bash

set -e

# AppVeyor and Drone Continuous Integration for MSYS2
# Author: Renato Silva <br.renatosilva@gmail.com>
# Author: Qian Hong <fracting@gmail.com>

# Configure
cd "$(dirname "$0")"
source 'ci-library.sh'
mkdir artifacts
git_config user.email 'ci@msys2.org'
git_config user.name  'MSYS2 Continuous Integration'
git remote add upstream 'https://github.com/MSYS2/MINGW-packages'
git fetch --quiet upstream
# So that makepkg auto-fetches keys from validpgpkeys
mkdir -p ~/.gnupg && echo -e "keyserver keyserver.ubuntu.com\nkeyserver-options auto-key-retrieve" > ~/.gnupg/gpg.conf
# reduce time required to install packages by disabling pacman's disk space checking
sed -i 's/^CheckSpace/#CheckSpace/g' /etc/pacman.conf

# Detect
list_commits  || failure 'Could not detect added commits'
list_packages || failure 'Could not detect changed files'
message 'Processing changes' "${commits[@]}"
test -z "${packages}" && success 'No changes in package recipes'
define_build_order || failure 'Could not determine build order'

# Build
message 'Building packages' "${packages[@]}"
execute 'Approving recipe quality' check_recipe_quality
for package in "${packages[@]}"; do
    execute 'Building binary' makepkg-mingw --noconfirm --noprogressbar --nocheck --syncdeps --rmdeps --cleanbuild
    execute 'Building source' makepkg --noconfirm --noprogressbar --allsource --config '/etc/makepkg_mingw64.conf'
    execute 'Installing' yes:pacman --noprogressbar --upgrade *"${PKGEXT}"
    mv "${package}"/*"${PKGEXT}" artifacts
    mv "${package}"/*"${SRCEXT}" artifacts
    unset package
done
success 'All packages built successfully'

cd artifacts
execute 'SHA-256 checksums' sha256sum *
