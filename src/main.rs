mod cli;
mod bridge;
mod output;
mod config;
mod error;

use cli::{parse_args, Commands};
use bridge::Engine;
use output::{OutputFormatter, json::JsonFormatter, terminal::TerminalFormatter, csv::CsvFormatter, html::HtmlFormatter};
use config::Config;
use colored::*;
use std::io::{self, Write};

fn main() {
    let args = parse_args();
    
    if args.verbose {
        println!("{}", "Byps - WAF Bypass Testing Tool".bold().green());
        println!("{}: {}\n", "Version".bold(), Engine::version());
    }
    
    let result = match args.command {
        Commands::Scan { url, techniques, strategy, output_file } => {
            handle_scan(&url, &techniques, &strategy, output_file.as_deref(), &args.output, args.verbose)
        }
        Commands::Test { url, technique } => {
            handle_test(&url, &technique, &args.output, args.verbose)
        }
        Commands::Detect { url } => {
            handle_detect(&url, args.verbose)
        }
        Commands::List { category } => {
            handle_list(category.as_deref())
        }
        Commands::Exploit { url, techniques, strategy, output_file, max_tests } => {
            handle_exploit(&url, &techniques, &strategy, output_file.as_deref(), &args.output, max_tests, args.verbose)
        }
        Commands::Wizard {} => {
            handle_wizard(&args.output, args.verbose)
        }
    };
    
    if let Err(e) = result {
        eprintln!("{}: {}", "Error".red().bold(), e);
        std::process::exit(1);
    }
}

fn handle_scan(url: &str, _techniques: &str, _strategy: &str, output_file: Option<&str>, output_format: &str, verbose: bool) -> error::Result<()> {
    if verbose {
        println!("{}: {}", "Scanning".bold(), url);
    }
    
    let engine = Engine::new()?;
    let config = Config::default();
    let config_json = config.to_json()?;
    
    let result = engine.scan(url, &config_json)?;
    
    match output_format {
        "json" => {
            let formatter = JsonFormatter;
            let formatted = formatter.format(&result)?;
            
            if let Some(file) = output_file {
                formatter.write_to_file(&result, file)?;
                if verbose {
                    println!("{}: {}", "Results written to".green(), file);
                }
            } else {
                println!("{}", formatted);
            }
        }
        "csv" => {
            let formatter = CsvFormatter;
            
            if let Some(file) = output_file {
                formatter.write_to_file(&result, file)?;
                if verbose {
                    println!("{}: {}", "Results written to".green(), file);
                }
            } else {
                let formatted = formatter.format(&result)?;
                println!("{}", formatted);
            }
        }
        "html" => {
            let formatter = HtmlFormatter;
            
            if let Some(file) = output_file {
                formatter.write_to_file(&result, file)?;
                if verbose {
                    println!("{}: {}", "HTML report written to".green(), file);
                }
            } else {
                let formatted = formatter.format(&result)?;
                println!("{}", formatted);
            }
        }
        "terminal" | _ => {
            let formatter = TerminalFormatter;
            formatter.print_colored(&result)?;
        }
    }
    
    Ok(())
}

fn handle_test(url: &str, technique: &str, output_format: &str, verbose: bool) -> error::Result<()> {
    if verbose {
        println!("{}: {} on {}", "Testing technique".bold(), technique.cyan(), url);
    }
    
    let engine = Engine::new()?;
    let result = engine.test_technique(url, technique)?;
    
    match output_format {
        "json" => {
            let formatter = JsonFormatter;
            let formatted = formatter.format(&result)?;
            println!("{}", formatted);
        }
        "csv" => {
            let formatter = CsvFormatter;
            let formatted = formatter.format(&result)?;
            println!("{}", formatted);
        }
        "html" => {
            let formatter = HtmlFormatter;
            let formatted = formatter.format(&result)?;
            println!("{}", formatted);
        }
        "terminal" | _ => {
            let formatter = TerminalFormatter;
            formatter.print_colored(&result)?;
        }
    }
    
    Ok(())
}

fn handle_detect(url: &str, verbose: bool) -> error::Result<()> {
    if verbose {
        println!("{}: {}", "Detecting WAF for".bold(), url);
    }
    
    println!("{}", "WAF detection not yet implemented".yellow());
    Ok(())
}

fn handle_list(category: Option<&str>) -> error::Result<()> {
    println!("{}", "Available Bypass Techniques:".bold().green());
    println!();
    
    let techniques = vec![
        ("path_bypass", "Path manipulation techniques (trailing slash, encoding, case variation)"),
        ("header_forge", "Header forgery (X-Forwarded-For, X-Original-URL, Host header)"),
        ("url_encoding", "URL encoding variations (single, double, triple encoding)"),
        ("unicode", "Unicode normalization and homograph substitution"),
        ("protocol_abuse", "HTTP protocol abuse (version manipulation, method variation)"),
    ];
    
    for (name, desc) in techniques {
        if let Some(cat) = category {
            if !name.contains(cat) {
                continue;
            }
        }
        println!("  {} - {}", name.cyan().bold(), desc);
    }
    
    println!();
    println!("{}", "Usage Examples:".bold().yellow());
    println!("  byps test https://example.com/admin --technique path_bypass");
    println!("  byps scan https://example.com/api -t path_bypass,header_forge");
    
    Ok(())
}

fn handle_exploit(url: &str, techniques: &str, strategy: &str, output_file: Option<&str>, output_format: &str, max_tests: usize, verbose: bool) -> error::Result<()> {
    println!("{}", "╔══════════════════════════════════════════════════════╗".bold().cyan());
    println!("{}", "║    🎯 AUTOMATED EXPLOITATION MODE ACTIVATED 🎯      ║".bold().cyan());
    println!("{}", "╚══════════════════════════════════════════════════════╝".bold().cyan());
    println!();
    
    if verbose {
        println!("{}: {}", "Target".bold(), url.yellow());
        println!("{}: {}", "Techniques".bold(), techniques.cyan());
        println!("{}: {}", "Strategy".bold(), strategy.magenta());
        println!("{}: {}", "Max Tests".bold(), max_tests.to_string().green());
        println!();
    }
    
    println!("{}", "Phase 1: Generating bypass variations...".bold());
    
    let engine = Engine::new()?;
    let config = Config::default();
    let config_json = config.to_json()?;
    
    let result = engine.scan(url, &config_json)?;
    
    // Parse JSON result to get variations
    let parsed: serde_json::Value = serde_json::from_str(&result)?;
    let variations = parsed.get("variations")
        .and_then(|v| v.as_array())
        .map(|arr| arr.len())
        .unwrap_or(0);
    
    println!("{} {} variations generated", "✓".green(), variations);
    println!();
    
    println!("{}", "Phase 2: Testing variations with HTTP requests...".bold());
    if verbose {
        println!("{}", "Sending real HTTP requests to test bypasses".yellow());
    }
    println!();
    
    // Actually test variations with HTTP requests
    let test_result = engine.test_variations(url, &config_json)?;
    
    // Parse test results
    let test_parsed: serde_json::Value = serde_json::from_str(&test_result)?;
    
    // Get baseline info
    let baseline_status = test_parsed.get("baseline")
        .and_then(|b| b.get("status"))
        .and_then(|s| s.as_i64())
        .unwrap_or(0) as i32;
    
    let baseline_size = test_parsed.get("baseline")
        .and_then(|b| b.get("size"))
        .and_then(|s| s.as_i64())
        .unwrap_or(0) as usize;
    
    if verbose {
        println!("Baseline request: status={}, size={} bytes", baseline_status, baseline_size);
        println!();
    }
    
    // Get test summary
    let total_tested = test_parsed.get("summary")
        .and_then(|s| s.get("total_tested"))
        .and_then(|t| t.as_i64())
        .unwrap_or(0) as usize;
    
    let successful_bypasses = test_parsed.get("summary")
        .and_then(|s| s.get("successful_bypasses"))
        .and_then(|t| t.as_i64())
        .unwrap_or(0) as usize;
    
    let failed_attempts = test_parsed.get("summary")
        .and_then(|s| s.get("failed_attempts"))
        .and_then(|t| t.as_i64())
        .unwrap_or(0) as usize;
    
    println!("Testing {} variations:", total_tested);
    
    // Show progress for first few tests
    let empty_vec = vec![];
    let variations_tested = test_parsed.get("variations")
        .and_then(|v| v.as_array())
        .unwrap_or(&empty_vec);
    
    for (i, var) in variations_tested.iter().take(std::cmp::min(5, total_tested)).enumerate() {
        let variation = var.get("variation").and_then(|v| v.as_str()).unwrap_or("?");
        let status = var.get("status").and_then(|s| s.as_i64()).unwrap_or(0);
        let is_bypass = var.get("bypass").and_then(|b| b.as_bool()).unwrap_or(false);
        
        if verbose {
            let marker = if is_bypass { "✓".green() } else { "→".cyan() };
            println!("  {} Testing variation {}/{}: {} -> {}", 
                     marker, i + 1, total_tested, variation, status);
        } else {
            println!("  {} Testing variation {}/{}", "→".cyan(), i + 1, total_tested);
        }
    }
    
    if total_tested > 5 {
        println!("  {} ... ({} more tests)", "→".cyan(), total_tested - 5);
    }
    println!();
    
    println!("{}", "Phase 3: Results Summary".bold().green());
    println!("  {} Total variations tested: {}", "•".cyan(), total_tested);
    println!("  {} Successful bypasses: {}", "•".green(), successful_bypasses);
    println!("  {} Failed attempts: {}", "•".red(), failed_attempts);
    println!();
    
    // Display successful bypasses
    if successful_bypasses > 0 {
        println!("{}", "Successful Bypasses:".bold().green());
        let mut bypass_count = 0;
        for var in variations_tested.iter() {
            let is_bypass = var.get("bypass").and_then(|b| b.as_bool()).unwrap_or(false);
            if is_bypass {
                let variation = var.get("variation").and_then(|v| v.as_str()).unwrap_or("?");
                let status = var.get("status").and_then(|s| s.as_i64()).unwrap_or(0);
                let reason = var.get("reason").and_then(|r| r.as_str()).unwrap_or("unknown");
                
                println!("  {} {} (status: {}, reason: {})", "✓".green(), variation, status, reason);
                bypass_count += 1;
                
                if bypass_count >= 10 && !verbose {
                    let remaining = successful_bypasses - bypass_count;
                    if remaining > 0 {
                        println!("  {} ... ({} more bypasses found)", "✓".green(), remaining);
                    }
                    break;
                }
            }
        }
    } else {
        println!("{}", "No successful bypasses found.".yellow());
        println!("{}", "  The target may not be vulnerable to these techniques.".yellow());
    }
    println!();
    
    // Save results if output file specified
    if let Some(file) = output_file {
        match output_format {
            "json" => {
                std::fs::write(file, &test_result)?;
                println!("{}: {}", "Exploits saved to".green(), file);
            }
            "csv" => {
                // Convert JSON to CSV format
                let formatter = CsvFormatter;
                formatter.write_to_file(&test_result, file)?;
                println!("{}: {}", "Exploits saved to".green(), file);
            }
            "html" => {
                let formatter = HtmlFormatter;
                formatter.write_to_file(&test_result, file)?;
                println!("{}: {}", "HTML report saved to".green(), file);
            }
            _ => {
                std::fs::write(file, &test_result)?;
                println!("{}: {}", "Results saved to".green(), file);
            }
        }
    }
    
    println!("{}", "╔══════════════════════════════════════════════════════╗".bold().green());
    println!("{}", "║         EXPLOITATION COMPLETE - CHECK RESULTS        ║".bold().green());
    println!("{}", "╚══════════════════════════════════════════════════════╝".bold().green());
    
    Ok(())
}

fn handle_wizard(output_format: &str, verbose: bool) -> error::Result<()> {
    println!("{}", "╔══════════════════════════════════════════════════════╗".bold().magenta());
    println!("{}", "║         🧙 BYPS INTERACTIVE WIZARD MODE 🧙          ║".bold().magenta());
    println!("{}", "╚══════════════════════════════════════════════════════╝".bold().magenta());
    println!();
    println!("{}", "Welcome to the Byps Interactive Wizard!".bold());
    println!("This wizard will guide you through the bypass testing process.\n");
    
    // Step 1: Get target URL
    print!("{} ", "Enter target URL (e.g., https://example.com/admin):".bold().cyan());
    io::stdout().flush()?;
    let mut url = String::new();
    io::stdin().read_line(&mut url)?;
    let url = url.trim().to_string();
    
    if url.is_empty() {
        return Err(error::BypsError::Unknown("URL cannot be empty".to_string()));
    }
    
    println!();
    
    // Step 2: Select mode
    println!("{}", "Select testing mode:".bold().cyan());
    println!("  {} Scan only (generate variations)", "1.".yellow());
    println!("  {} Exploit (automated testing)", "2.".yellow());
    print!("\n{} ", "Enter choice [1-2]:".bold().cyan());
    io::stdout().flush()?;
    let mut mode = String::new();
    io::stdin().read_line(&mut mode)?;
    let mode = mode.trim();
    
    println!();
    
    // Step 3: Select techniques
    println!("{}", "Select bypass techniques:".bold().cyan());
    println!("  {} path_bypass    - Path manipulation", "1.".yellow());
    println!("  {} header_forge   - Header forgery", "2.".yellow());
    println!("  {} url_encoding   - URL encoding", "3.".yellow());
    println!("  {} unicode        - Unicode tricks", "4.".yellow());
    println!("  {} protocol_abuse - Protocol manipulation", "5.".yellow());
    println!("  {} all            - Use all techniques", "6.".yellow());
    print!("\n{} ", "Enter choice [1-6]:".bold().cyan());
    io::stdout().flush()?;
    let mut tech_choice = String::new();
    io::stdin().read_line(&mut tech_choice)?;
    let techniques = match tech_choice.trim() {
        "1" => "path_bypass",
        "2" => "header_forge",
        "3" => "url_encoding",
        "4" => "unicode",
        "5" => "protocol_abuse",
        _ => "all",
    };
    
    println!();
    
    // Step 4: Select strategy
    println!("{}", "Select scan strategy:".bold().cyan());
    println!("  {} fast      - Quick scan (5-10 variations)", "1.".yellow());
    println!("  {} balanced  - Standard scan (10-20 variations)", "2.".yellow());
    println!("  {} thorough  - Comprehensive scan (20+ variations)", "3.".yellow());
    println!("  {} stealth   - Slow, careful scan", "4.".yellow());
    print!("\n{} ", "Enter choice [1-4]:".bold().cyan());
    io::stdout().flush()?;
    let mut strat_choice = String::new();
    io::stdin().read_line(&mut strat_choice)?;
    let strategy = match strat_choice.trim() {
        "1" => "fast",
        "3" => "thorough",
        "4" => "stealth",
        _ => "balanced",
    };
    
    println!();
    
    // Step 5: Save to file?
    print!("{} ", "Save results to file? (y/N):".bold().cyan());
    io::stdout().flush()?;
    let mut save_choice = String::new();
    io::stdin().read_line(&mut save_choice)?;
    let save_to_file = save_choice.trim().to_lowercase() == "y";
    
    let output_file = if save_to_file {
        print!("{} ", "Enter output filename:".bold().cyan());
        io::stdout().flush()?;
        let mut filename = String::new();
        io::stdin().read_line(&mut filename)?;
        Some(filename.trim().to_string())
    } else {
        None
    };
    
    println!();
    println!("{}", "═".repeat(60).cyan());
    println!("{}", "Configuration Summary".bold().green());
    println!("{}", "═".repeat(60).cyan());
    println!("  {}: {}", "Target URL".bold(), url.yellow());
    println!("  {}: {}", "Mode".bold(), if mode == "2" { "Exploit" } else { "Scan" }.cyan());
    println!("  {}: {}", "Techniques".bold(), techniques.cyan());
    println!("  {}: {}", "Strategy".bold(), strategy.magenta());
    if let Some(ref file) = output_file {
        println!("  {}: {}", "Output File".bold(), file.green());
    }
    println!("{}", "═".repeat(60).cyan());
    println!();
    
    print!("{} ", "Proceed with these settings? (Y/n):".bold().green());
    io::stdout().flush()?;
    let mut confirm = String::new();
    io::stdin().read_line(&mut confirm)?;
    
    if confirm.trim().to_lowercase() == "n" {
        println!("{}", "Operation cancelled.".yellow());
        return Ok(());
    }
    
    println!();
    println!("{}", "Starting test...".bold().green());
    println!();
    
    // Execute the selected mode
    if mode == "2" {
        handle_exploit(&url, techniques, strategy, output_file.as_deref(), output_format, 50, verbose)
    } else {
        handle_scan(&url, techniques, strategy, output_file.as_deref(), output_format, verbose)
    }
}
