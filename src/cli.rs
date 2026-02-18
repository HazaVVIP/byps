use clap::{Parser, Subcommand};

#[derive(Parser)]
#[command(name = "byps")]
#[command(author = "HazaVVIP")]
#[command(version = "0.1.0")]
#[command(about = "CLI bypass tool for WAF and access control testing", long_about = None)]
pub struct Cli {
    #[command(subcommand)]
    pub command: Commands,
    
    /// Enable verbose output
    #[arg(short, long, global = true)]
    pub verbose: bool,
    
    /// Output format
    #[arg(short, long, value_name = "FORMAT", default_value = "json", global = true)]
    pub output: String,
}

#[derive(Subcommand)]
pub enum Commands {
    /// Scan a URL for bypass opportunities
    Scan {
        /// Target URL to scan
        #[arg(value_name = "URL")]
        url: String,
        
        /// Techniques to use (comma-separated)
        #[arg(short, long, value_name = "TECHNIQUES", default_value = "all")]
        techniques: String,
        
        /// Scan strategy (fast, balanced, thorough, stealth)
        #[arg(short, long, value_name = "STRATEGY", default_value = "balanced")]
        strategy: String,
        
        /// Output file
        #[arg(short = 'o', long, value_name = "FILE")]
        output_file: Option<String>,
    },
    
    /// Test a specific bypass technique
    Test {
        /// Target URL to test
        #[arg(value_name = "URL")]
        url: String,
        
        /// Technique name
        #[arg(short, long, value_name = "TECHNIQUE")]
        technique: String,
    },
    
    /// Detect WAF type
    Detect {
        /// Target URL to analyze
        #[arg(value_name = "URL")]
        url: String,
    },
    
    /// List available techniques
    List {
        /// Filter by category
        #[arg(short, long, value_name = "CATEGORY")]
        category: Option<String>,
    },
}

pub fn parse_args() -> Cli {
    Cli::parse()
}
