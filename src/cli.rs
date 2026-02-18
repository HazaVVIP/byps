use clap::{Parser, Subcommand};

const LONG_ABOUT: &str = "\
Byps - Advanced WAF Bypass Testing Tool

A high-performance CLI tool for testing and bypassing Web Application Firewalls (WAFs)
and access control restrictions using multiple bypass techniques.

FEATURES:
  • Multiple bypass techniques (path, header, encoding, unicode, protocol)
  • Automated exploitation of detected bypasses
  • Interactive wizard for guided testing
  • Multiple output formats (JSON, CSV, HTML, Terminal)
  • Configurable scan strategies (fast, balanced, thorough, stealth)

EXAMPLES:
  # Quick scan with default settings
  byps scan https://example.com/admin

  # Scan with specific techniques
  byps scan https://example.com/api -t path_bypass,header_forge

  # Test single technique
  byps test https://example.com/secret --technique url_encoding

  # Automated exploitation mode
  byps exploit https://example.com/admin

  # Interactive wizard mode
  byps wizard

  # List all available techniques
  byps list

For more information on each command, use: byps <command> --help";

#[derive(Parser)]
#[command(name = "byps")]
#[command(author = "HazaVVIP")]
#[command(version = "0.1.0")]
#[command(about = "Advanced WAF bypass testing tool")]
#[command(long_about = LONG_ABOUT)]
#[command(after_help = "Use 'byps <command> --help' for more information on a specific command.")]
pub struct Cli {
    #[command(subcommand)]
    pub command: Commands,
    
    /// Enable verbose output with detailed logging
    #[arg(short, long, global = true)]
    pub verbose: bool,
    
    /// Output format: json, csv, html, terminal
    #[arg(short, long, value_name = "FORMAT", default_value = "terminal", global = true)]
    pub output: String,
}

#[derive(Subcommand)]
pub enum Commands {
    /// Scan a URL for bypass opportunities
    #[command(long_about = "\
Scan a target URL for potential bypass opportunities using multiple techniques.

This command generates various bypass variations and displays them for manual testing.
Use the 'exploit' command to automatically test the variations.

STRATEGIES:
  • fast      - Quick scan with common bypasses (5-10 variations per technique)
  • balanced  - Standard scan with good coverage (10-20 variations) [default]
  • thorough  - Comprehensive scan with maximum variations (20+ per technique)
  • stealth   - Slow, low-profile scan to avoid detection (limited variations)

TECHNIQUES:
  • path_bypass    - Path manipulation (trailing slash, encoding, traversal)
  • header_forge   - Header forgery (X-Forwarded-For, X-Original-URL)
  • url_encoding   - URL encoding variations (single, double, triple)
  • unicode        - Unicode normalization and homographs
  • protocol_abuse - HTTP protocol abuse (method, version manipulation)
  • all           - Use all available techniques [default]

EXAMPLES:
  # Scan with default settings
  byps scan https://example.com/admin

  # Fast scan with specific techniques
  byps scan https://example.com/api -t path_bypass,header_forge -s fast

  # Thorough scan with HTML output
  byps scan https://example.com/secret -s thorough --output html -o report.html

  # Stealth scan with verbose output
  byps scan https://example.com/admin -s stealth -v")]
    Scan {
        /// Target URL to scan (e.g., https://example.com/admin)
        #[arg(value_name = "URL")]
        url: String,
        
        /// Techniques to use: comma-separated list or 'all'
        #[arg(short, long, value_name = "TECHNIQUES", default_value = "all")]
        techniques: String,
        
        /// Scan strategy: fast, balanced, thorough, stealth
        #[arg(short, long, value_name = "STRATEGY", default_value = "balanced")]
        strategy: String,
        
        /// Output file path
        #[arg(short = 'f', long, value_name = "FILE")]
        output_file: Option<String>,
    },
    
    /// Test a specific bypass technique
    #[command(long_about = "\
Test a single bypass technique against a target URL.

This is useful for focused testing when you want to see variations from
a specific technique without running a full scan.

AVAILABLE TECHNIQUES:
  • path_bypass    - Path manipulation techniques
  • header_forge   - Header forgery methods
  • url_encoding   - URL encoding variations
  • unicode        - Unicode-based bypasses
  • protocol_abuse - Protocol manipulation

EXAMPLES:
  # Test path bypass technique
  byps test https://example.com/admin --technique path_bypass

  # Test URL encoding with JSON output
  byps test https://example.com/api -t url_encoding --output json")]
    Test {
        /// Target URL to test (e.g., https://example.com/admin)
        #[arg(value_name = "URL")]
        url: String,
        
        /// Technique name to test
        #[arg(short, long, value_name = "TECHNIQUE")]
        technique: String,
    },
    
    /// Detect WAF type and version
    #[command(long_about = "\
Detect the type and version of Web Application Firewall protecting the target.

This command analyzes response headers, status codes, and behavior patterns
to identify the WAF vendor and version.

EXAMPLES:
  # Detect WAF
  byps detect https://example.com

  # Detect with verbose output
  byps detect https://example.com -v")]
    Detect {
        /// Target URL to analyze (e.g., https://example.com)
        #[arg(value_name = "URL")]
        url: String,
    },
    
    /// List available bypass techniques
    #[command(long_about = "\
List all available bypass techniques with descriptions.

EXAMPLES:
  # List all techniques
  byps list

  # Filter by category
  byps list -c path")]
    List {
        /// Filter by category keyword
        #[arg(short, long, value_name = "CATEGORY")]
        category: Option<String>,
    },
    
    /// Automated exploitation mode - test bypass variations automatically
    #[command(long_about = "\
Automated exploitation mode that scans for bypasses and automatically tests them.

This mode combines scanning and live testing:
  1. Generates bypass variations using all techniques
  2. Automatically sends HTTP requests to test each variation
  3. Compares responses to detect successful bypasses
  4. Reports working bypasses and saves them

This is more aggressive than 'scan' mode and will generate real traffic to the target.

STRATEGIES:
  • fast      - Quick automated testing (5-10 variations per technique)
  • balanced  - Standard automated testing (10-20 variations) [default]
  • thorough  - Comprehensive automated testing (20+ variations)
  • stealth   - Slow, careful automated testing with delays

EXAMPLES:
  # Automated exploitation with default settings
  byps exploit https://example.com/admin

  # Fast exploitation with specific techniques
  byps exploit https://example.com/api -t path_bypass,header_forge -s fast

  # Thorough exploitation with results saved to file
  byps exploit https://example.com/secret -s thorough -o exploits.json

  # Stealth exploitation to avoid detection
  byps exploit https://example.com/admin -s stealth -v")]
    Exploit {
        /// Target URL to exploit (e.g., https://example.com/admin)
        #[arg(value_name = "URL")]
        url: String,
        
        /// Techniques to use: comma-separated list or 'all'
        #[arg(short, long, value_name = "TECHNIQUES", default_value = "all")]
        techniques: String,
        
        /// Exploitation strategy: fast, balanced, thorough, stealth
        #[arg(short, long, value_name = "STRATEGY", default_value = "balanced")]
        strategy: String,
        
        /// Output file for successful exploits
        #[arg(short = 'f', long, value_name = "FILE")]
        output_file: Option<String>,
        
        /// Maximum number of variations to test (0 = unlimited)
        #[arg(short = 'm', long, value_name = "MAX", default_value = "50")]
        max_tests: usize,
    },
    
    /// Interactive wizard mode for guided bypass testing
    #[command(long_about = "\
Interactive wizard mode that guides you through the bypass testing process.

This user-friendly mode is perfect for beginners or when you want a guided
experience. The wizard will:
  1. Ask for the target URL
  2. Help you select techniques and strategy
  3. Choose output format and file
  4. Run the scan/exploitation
  5. Display results in a user-friendly format

EXAMPLES:
  # Start interactive wizard
  byps wizard

  # Wizard with verbose output
  byps wizard -v")]
    Wizard {
        // No arguments needed - wizard is interactive
    },
    
    /// Update byps to the latest version
    #[command(long_about = "\
Update byps to the latest version from the repository.

This command will:
  1. Check for the latest version available
  2. Download and install the update if available
  3. Display the changelog and new features

EXAMPLES:
  # Check for updates and install
  byps update

  # Check for updates with verbose output
  byps update -v")]
    Update {
        // No arguments needed for update
    },
}

pub fn parse_args() -> Cli {
    Cli::parse()
}
