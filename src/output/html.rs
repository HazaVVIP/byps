use crate::output::OutputFormatter;
use crate::error::Result;
use serde_json::Value;
use std::fs;

pub struct HtmlFormatter;

impl HtmlFormatter {
    fn json_to_html(&self, data: &str) -> Result<String> {
        let value: Value = serde_json::from_str(data)?;
        
        let mut html = String::from(r#"<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Byps Scan Report</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            padding: 20px;
            min-height: 100vh;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            border-radius: 10px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.2);
            overflow: hidden;
        }
        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            text-align: center;
        }
        .header h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
        }
        .header p {
            font-size: 1.1em;
            opacity: 0.9;
        }
        .content {
            padding: 30px;
        }
        .section {
            margin-bottom: 30px;
        }
        .section h2 {
            color: #667eea;
            border-bottom: 2px solid #667eea;
            padding-bottom: 10px;
            margin-bottom: 20px;
        }
        .info-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-bottom: 20px;
        }
        .info-card {
            background: #f8f9fa;
            padding: 15px;
            border-radius: 8px;
            border-left: 4px solid #667eea;
        }
        .info-card .label {
            font-weight: bold;
            color: #555;
            margin-bottom: 5px;
        }
        .info-card .value {
            color: #333;
            word-break: break-all;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 15px;
        }
        th {
            background: #667eea;
            color: white;
            padding: 12px;
            text-align: left;
            font-weight: 600;
        }
        td {
            padding: 12px;
            border-bottom: 1px solid #e0e0e0;
        }
        tr:hover {
            background: #f5f5f5;
        }
        .status-success {
            color: #28a745;
            font-weight: bold;
        }
        .status-error {
            color: #dc3545;
            font-weight: bold;
        }
        .status-unknown {
            color: #6c757d;
            font-weight: bold;
        }
        .footer {
            background: #f8f9fa;
            padding: 20px;
            text-align: center;
            color: #666;
            border-top: 1px solid #e0e0e0;
        }
        .timestamp {
            font-size: 0.9em;
            color: #888;
        }
        code {
            background: #f4f4f4;
            padding: 2px 6px;
            border-radius: 3px;
            font-family: 'Courier New', monospace;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🔓 Byps Scan Report</h1>
            <p>WAF Bypass Testing Results</p>
        </div>
        <div class="content">
"#);

        if let Some(obj) = value.as_object() {
            // Summary section
            html.push_str(r#"            <div class="section">
                <h2>Summary</h2>
                <div class="info-grid">
"#);
            
            if let Some(url) = obj.get("url").and_then(|v| v.as_str()) {
                html.push_str(&format!(r#"                    <div class="info-card">
                        <div class="label">Target URL</div>
                        <div class="value"><code>{}</code></div>
                    </div>
"#, Self::escape_html(url)));
            }
            
            if let Some(variations) = obj.get("variations").and_then(|v| v.as_array()) {
                html.push_str(&format!(r#"                    <div class="info-card">
                        <div class="label">Variations Found</div>
                        <div class="value">{}</div>
                    </div>
"#, variations.len()));
            }
            
            if let Some(timestamp) = obj.get("timestamp").and_then(|v| v.as_str()) {
                html.push_str(&format!(r#"                    <div class="info-card">
                        <div class="label">Scan Time</div>
                        <div class="value">{}</div>
                    </div>
"#, Self::escape_html(timestamp)));
            }
            
            html.push_str("                </div>\n            </div>\n");
            
            // Variations section
            if let Some(variations) = obj.get("variations").and_then(|v| v.as_array()) {
                html.push_str(r#"            <div class="section">
                <h2>Bypass Variations</h2>
                <table>
                    <thead>
                        <tr>
                            <th>#</th>
                            <th>Variation</th>
                            <th>Technique</th>
                            <th>Status</th>
                        </tr>
                    </thead>
                    <tbody>
"#);
                
                for (i, var) in variations.iter().enumerate() {
                    let (url, technique, status) = if let Some(o) = var.as_object() {
                        let u = o.get("url").and_then(|v| v.as_str()).unwrap_or("unknown");
                        let t = o.get("technique").and_then(|v| v.as_str()).unwrap_or("unknown");
                        let s = o.get("status").and_then(|v| v.as_str()).unwrap_or("unknown");
                        (u, t, s)
                    } else if let Some(s) = var.as_str() {
                        (s, "variation", "unknown")
                    } else {
                        ("unknown", "unknown", "unknown")
                    };
                    
                    let status_class = match status {
                        "success" => "status-success",
                        "error" | "failed" => "status-error",
                        _ => "status-unknown",
                    };
                    
                    html.push_str(&format!(r#"                        <tr>
                            <td>{}</td>
                            <td><code>{}</code></td>
                            <td>{}</td>
                            <td class="{}">{}</td>
                        </tr>
"#, i + 1, 
                        Self::escape_html(url),
                        Self::escape_html(technique),
                        status_class,
                        Self::escape_html(status)));
                }
                
                html.push_str(r#"                    </tbody>
                </table>
            </div>
"#);
            }
            
            // Additional metadata
            let mut other_fields = Vec::new();
            for (key, val) in obj {
                if key != "url" && key != "variations" && key != "timestamp" {
                    other_fields.push((key, val));
                }
            }
            
            if !other_fields.is_empty() {
                html.push_str(r#"            <div class="section">
                <h2>Additional Information</h2>
                <div class="info-grid">
"#);
                
                for (key, val) in other_fields {
                    let value_str = match val {
                        Value::String(s) => s.clone(),
                        Value::Number(n) => n.to_string(),
                        Value::Bool(b) => b.to_string(),
                        _ => serde_json::to_string_pretty(val).unwrap_or_default(),
                    };
                    
                    html.push_str(&format!(r#"                    <div class="info-card">
                        <div class="label">{}</div>
                        <div class="value">{}</div>
                    </div>
"#, Self::escape_html(key), Self::escape_html(&value_str)));
                }
                
                html.push_str("                </div>\n            </div>\n");
            }
        }
        
        html.push_str(r#"        </div>
        <div class="footer">
            <p>Generated by <strong>Byps</strong> - WAF Bypass Testing Tool</p>
            <p class="timestamp">Report generated at: <span id="reportTime"></span></p>
        </div>
    </div>
    <script>
        document.getElementById('reportTime').textContent = new Date().toLocaleString();
    </script>
</body>
</html>
"#);
        
        Ok(html)
    }
    
    fn escape_html(s: &str) -> String {
        s.replace('&', "&amp;")
            .replace('<', "&lt;")
            .replace('>', "&gt;")
            .replace('"', "&quot;")
            .replace('\'', "&#39;")
    }
}

impl OutputFormatter for HtmlFormatter {
    fn format(&self, data: &str) -> Result<String> {
        self.json_to_html(data)
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
    fn test_escape_html() {
        assert_eq!(HtmlFormatter::escape_html("simple"), "simple");
        assert_eq!(HtmlFormatter::escape_html("<script>"), "&lt;script&gt;");
        assert_eq!(HtmlFormatter::escape_html("a&b"), "a&amp;b");
    }

    #[test]
    fn test_format_creates_valid_html() {
        let formatter = HtmlFormatter;
        let json = r#"{"url":"http://example.com","variations":[]}"#;
        let result = formatter.format(json);
        assert!(result.is_ok());
        let html = result.unwrap();
        assert!(html.contains("<!DOCTYPE html>"));
        assert!(html.contains("</html>"));
    }
}
