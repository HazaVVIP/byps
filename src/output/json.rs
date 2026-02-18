use crate::output::OutputFormatter;
use crate::error::Result;
use serde_json::Value;
use std::fs;

pub struct JsonFormatter;

impl OutputFormatter for JsonFormatter {
    fn format(&self, data: &str) -> Result<String> {
        // Parse and pretty-print JSON
        let value: Value = serde_json::from_str(data)?;
        let formatted = serde_json::to_string_pretty(&value)?;
        Ok(formatted)
    }
    
    fn write_to_file(&self, data: &str, path: &str) -> Result<()> {
        let formatted = self.format(data)?;
        fs::write(path, formatted)?;
        Ok(())
    }
}
