use crate::output::OutputFormatter;
use crate::error::Result;
use colored::*;
use serde_json::Value;
use std::fs;

pub struct TerminalFormatter;

impl TerminalFormatter {
    pub fn print_colored(&self, data: &str) -> Result<()> {
        let value: Value = serde_json::from_str(data)?;
        
        if let Some(obj) = value.as_object() {
            println!("{}", "=== Scan Results ===".bold().green());
            
            if let Some(url) = obj.get("url") {
                println!("{}: {}", "Target URL".bold(), url);
            }
            
            if let Some(variations) = obj.get("variations").and_then(|v| v.as_array()) {
                println!("\n{}: {}", "Variations Found".bold(), variations.len());
                for (i, var) in variations.iter().enumerate() {
                    if let Some(s) = var.as_str() {
                        println!("  {}. {}", i + 1, s.cyan());
                    }
                }
            }
        }
        
        Ok(())
    }
}

impl OutputFormatter for TerminalFormatter {
    fn format(&self, data: &str) -> Result<String> {
        Ok(data.to_string())
    }
    
    fn write_to_file(&self, data: &str, path: &str) -> Result<()> {
        fs::write(path, data)?;
        Ok(())
    }
}
