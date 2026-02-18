#!/bin/bash
# Byps Installer Script
# Automated installation for WAF Bypass Testing Tool

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
INSTALL_DIR="/usr/local/bin"
BUILD_DIR="build"
REPO_URL="https://github.com/HazaVVIP/byps.git"
REPO_DIR="$HOME/.byps"

# Print colored messages
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Check if running as root for system-wide installation
check_permissions() {
    if [ "$EUID" -ne 0 ]; then
        print_warning "This script is not running as root."
        print_info "The installer will attempt to use sudo for system-wide installation."
        SUDO="sudo"
    else
        SUDO=""
    fi
}

# Detect OS and set package manager
detect_os() {
    print_info "Detecting operating system..."
    
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        OS=$ID
        print_success "Detected OS: $OS"
    elif [ "$(uname)" == "Darwin" ]; then
        OS="macos"
        print_success "Detected OS: macOS"
    else
        print_error "Unable to detect operating system"
        exit 1
    fi
}

# Install dependencies based on OS
install_dependencies() {
    print_info "Installing dependencies..."
    
    case $OS in
        ubuntu|debian|linuxmint|pop)
            print_info "Using apt package manager..."
            $SUDO apt-get update
            $SUDO apt-get install -y cmake build-essential libssl-dev pkg-config curl git
            ;;
        fedora|rhel|centos)
            print_info "Using dnf/yum package manager..."
            if command -v dnf &> /dev/null; then
                $SUDO dnf install -y cmake gcc gcc-c++ openssl-devel pkg-config curl git
            else
                $SUDO yum install -y cmake gcc gcc-c++ openssl-devel pkg-config curl git
            fi
            ;;
        arch|manjaro)
            print_info "Using pacman package manager..."
            $SUDO pacman -Sy --noconfirm cmake base-devel openssl pkg-config curl git
            ;;
        macos)
            print_info "Using Homebrew package manager..."
            if ! command -v brew &> /dev/null; then
                print_error "Homebrew is not installed. Please install it from https://brew.sh"
                exit 1
            fi
            brew install cmake openssl pkg-config curl git
            ;;
        *)
            print_error "Unsupported operating system: $OS"
            print_info "Please install the following dependencies manually:"
            print_info "  - CMake 3.15+"
            print_info "  - C++17 compiler (g++/clang)"
            print_info "  - OpenSSL development libraries"
            print_info "  - pkg-config"
            print_info "  - curl"
            print_info "  - git"
            exit 1
            ;;
    esac
    
    print_success "Dependencies installed successfully"
}

# Install Rust if not present
install_rust() {
    if command -v cargo &> /dev/null; then
        print_success "Rust is already installed ($(cargo --version))"
        
        # Check Rust version
        RUST_VERSION=$(cargo --version | awk '{print $2}')
        REQUIRED_VERSION="1.70.0"
        
        if [ "$(printf '%s\n' "$REQUIRED_VERSION" "$RUST_VERSION" | sort -V | head -n1)" != "$REQUIRED_VERSION" ]; then
            print_warning "Rust version $RUST_VERSION is below recommended $REQUIRED_VERSION"
            print_info "Updating Rust..."
            rustup update
        fi
    else
        print_info "Installing Rust..."
        curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
        source "$HOME/.cargo/env"
        print_success "Rust installed successfully"
    fi
}

# Clone or update repository
setup_repository() {
    if [ -d "$REPO_DIR" ]; then
        print_info "Repository already exists at $REPO_DIR"
        print_info "Updating repository..."
        cd "$REPO_DIR"
        git fetch origin
        git reset --hard origin/main
        git pull origin main
        print_success "Repository updated successfully"
    else
        print_info "Cloning repository to $REPO_DIR..."
        git clone "$REPO_URL" "$REPO_DIR"
        cd "$REPO_DIR"
        print_success "Repository cloned successfully"
    fi
}

# Build C++ core
build_cpp_core() {
    print_info "Building C++ core engine..."
    
    cd "$REPO_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    cmake .. -DCMAKE_BUILD_TYPE=Release
    
    # Determine number of CPU cores
    if [ "$(uname)" == "Darwin" ]; then
        CORES=$(sysctl -n hw.ncpu)
    else
        CORES=$(nproc)
    fi
    
    make -j"$CORES"
    
    print_success "C++ core built successfully"
    cd "$REPO_DIR"
}

# Build Rust CLI
build_rust_cli() {
    print_info "Building Rust CLI..."
    
    cd "$REPO_DIR"
    
    # Ensure cargo is in PATH
    if ! command -v cargo &> /dev/null; then
        source "$HOME/.cargo/env"
    fi
    
    cargo build --release
    
    print_success "Rust CLI built successfully"
}

# Install binary
install_binary() {
    print_info "Installing byps to $INSTALL_DIR..."
    
    if [ ! -f "$REPO_DIR/target/release/byps" ]; then
        print_error "Binary not found at $REPO_DIR/target/release/byps"
        exit 1
    fi
    
    $SUDO cp "$REPO_DIR/target/release/byps" "$INSTALL_DIR/byps"
    $SUDO chmod +x "$INSTALL_DIR/byps"
    
    print_success "Byps installed to $INSTALL_DIR/byps"
}

# Verify installation
verify_installation() {
    print_info "Verifying installation..."
    
    if command -v byps &> /dev/null; then
        VERSION=$(byps --version 2>&1 || echo "unknown")
        print_success "Byps is installed and accessible in PATH"
        print_info "Version: $VERSION"
    else
        print_error "Byps command not found in PATH"
        print_info "You may need to add $INSTALL_DIR to your PATH"
        exit 1
    fi
}

# Uninstall function
uninstall() {
    print_info "Uninstalling byps..."
    
    if [ -f "$INSTALL_DIR/byps" ]; then
        $SUDO rm "$INSTALL_DIR/byps"
        print_success "Binary removed from $INSTALL_DIR"
    fi
    
    if [ -d "$REPO_DIR" ]; then
        print_info "Removing repository at $REPO_DIR..."
        read -p "Are you sure you want to remove $REPO_DIR? (y/N): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            rm -rf "$REPO_DIR"
            print_success "Repository removed"
        else
            print_info "Keeping repository at $REPO_DIR"
        fi
    fi
    
    print_success "Byps uninstalled successfully"
}

# Main installation function
main() {
    echo ""
    echo "========================================"
    echo "  Byps - WAF Bypass Testing Tool"
    echo "  Automated Installer"
    echo "========================================"
    echo ""
    
    # Check for uninstall flag
    if [ "$1" == "--uninstall" ] || [ "$1" == "-u" ]; then
        check_permissions
        uninstall
        exit 0
    fi
    
    # Check for help flag
    if [ "$1" == "--help" ] || [ "$1" == "-h" ]; then
        echo "Usage: $0 [OPTIONS]"
        echo ""
        echo "Options:"
        echo "  (no options)    Install byps"
        echo "  --uninstall, -u Uninstall byps"
        echo "  --help, -h      Show this help message"
        echo ""
        exit 0
    fi
    
    print_warning "This installer will:"
    print_info "  1. Install system dependencies (requires sudo)"
    print_info "  2. Install Rust (if not present)"
    print_info "  3. Clone/update byps repository to $REPO_DIR"
    print_info "  4. Build the C++ core and Rust CLI"
    print_info "  5. Install binary to $INSTALL_DIR"
    echo ""
    
    read -p "Continue with installation? (Y/n): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]] && [[ ! -z $REPLY ]]; then
        print_info "Installation cancelled"
        exit 0
    fi
    
    echo ""
    
    # Run installation steps
    check_permissions
    detect_os
    install_dependencies
    install_rust
    setup_repository
    build_cpp_core
    build_rust_cli
    install_binary
    verify_installation
    
    echo ""
    echo "========================================"
    print_success "Installation Complete!"
    echo "========================================"
    echo ""
    print_info "Quick Start:"
    echo "  byps --help                           # Show help"
    echo "  byps list                             # List bypass techniques"
    echo "  byps scan <url>                       # Scan a URL"
    echo "  byps wizard                           # Interactive mode"
    echo ""
    print_warning "Legal Disclaimer:"
    print_info "This tool is for AUTHORIZED TESTING ONLY."
    print_info "Use only on systems you own or have permission to test."
    echo ""
}

# Run main function
main "$@"
