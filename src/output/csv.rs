use crate::output::OutputFormatter;
use crate::error::Result;
use serde_json::Value;
use std::fs;

pub struct CsvFormatter;

impl CsvFormatter {
    fn json_to_csv(&self, data: &str) -> Result<String> {
        let value: Value = serde_json::from_str(data)?;
        let mut csv = String::new();
        
        // Handle different JSON structures
        if let Some(obj) = value.as_object() {
            // Check if it's a scan result
            if let Some(variations) = obj.get("variations").and_then(|v| v.as_array()) {
                csv.push_str("Index,URL,Technique,Status\n");
                
                for (i, var) in variations.iter().enumerate() {
                    let var_str = if let Some(s) = var.as_str() {
                        s
                    } else if let Some(o) = var.as_object() {
                        o.get("url")
                            .and_then(|v| v.as_str())
                            .unwrap_or("unknown")
                    } else {
                        "unknown"
                    };
                    
                    let technique = if let Some(o) = var.as_object() {
                        o.get("technique")
                            .and_then(|v| v.as_str())
                            .unwrap_or("unknown")
                    } else {
                        "variation"
                    };
                    
                    let status = if let Some(o) = var.as_object() {
                        o.get("status")
                            .and_then(|v| v.as_str())
                            .unwrap_or("unknown")
                    } else {
                        "unknown"
                    };
                    
                    csv.push_str(&format!("{},{},{},{}\n", i + 1, 
                        Self::escape_csv(var_str),
                        Self::escape_csv(technique),
                        Self::escape_csv(status)));
                }
            } else {
                // Generic object to CSV
                csv.push_str("Key,Value\n");
                for (key, val) in obj {
                    let value_str = match val {
                        Value::String(s) => s.clone(),
                        Value::Number(n) => n.to_string(),
                        Value::Bool(b) => b.to_string(),
                        _ => serde_json::to_string(val)?,
                    };
                    csv.push_str(&format!("{},{}\n", 
                        Self::escape_csv(key),
                        Self::escape_csv(&value_str)));
                }
            }
        } else if let Some(arr) = value.as_array() {
            // Array of items
            if arr.is_empty() {
                csv.push_str("No data\n");
            } else {
                csv.push_str("Index,Value\n");
                for (i, item) in arr.iter().enumerate() {
                    let item_str = match item {
                        Value::String(s) => s.clone(),
                        _ => serde_json::to_string(item)?,
                    };
                    csv.push_str(&format!("{},{}\n", i + 1, Self::escape_csv(&item_str)));
                }
            }
        }
        
        Ok(csv)
    }
    
    fn escape_csv(s: &str) -> String {
        if s.contains(',') || s.contains('"') || s.contains('\n') {
            format!("\"{}\"", s.replace('"', "\"\""))
        } else {
            s.to_string()
        }
    }
}

impl OutputFormatter for CsvFormatter {
    fn format(&self, data: &str) -> Result<String> {
        self.json_to_csv(data)
    }
    
    fn write_to_file(&self, data: &str, path: &str) -> Result<()> {
        let formatted = self.format(data)?;
        fs::write(path, formatted)?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_escape_csv() {
        assert_eq!(CsvFormatter::escape_csv("simple"), "simple");
        assert_eq!(CsvFormatter::escape_csv("with,comma"), "\"with,comma\"");
        assert_eq!(CsvFormatter::escape_csv("with\"quote"), "\"with\"\"quote\"");
    }

    #[test]
    fn test_format_simple_object() {
        let formatter = CsvFormatter;
        let json = r#"{"url":"http://example.com","status":"success"}"#;
        let result = formatter.format(json);
        assert!(result.is_ok());
        let csv = result.unwrap();
        assert!(csv.contains("Key,Value"));
    }
}
