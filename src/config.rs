use serde::{Deserialize, Serialize};
use crate::error::Result;

#[derive(Debug, Serialize, Deserialize)]
pub struct Config {
    pub timeout: u32,
    pub retries: u32,
    pub concurrent: u32,
    pub verbosity: String,
    pub techniques: Vec<String>,
    pub strategy: String,
}

impl Default for Config {
    fn default() -> Self {
        Config {
            timeout: 10,
            retries: 3,
            concurrent: 10,
            verbosity: "info".to_string(),
            techniques: vec!["path_bypass".to_string(), "header_forge".to_string()],
            strategy: "balanced".to_string(),
        }
    }
}

impl Config {
    pub fn to_json(&self) -> Result<String> {
        Ok(serde_json::to_string(self)?)
    }
}
