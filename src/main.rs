mod cli;
mod bridge;
mod output;
mod config;
mod error;

use cli::{parse_args, Commands};
use bridge::Engine;
use output::{OutputFormatter, json::JsonFormatter, terminal::TerminalFormatter};
use config::Config;
use colored::*;

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
    
    Ok(())
}
