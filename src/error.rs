use thiserror::Error;

#[derive(Error, Debug)]
pub enum BypsError {
    #[error("Network error: {0}")]
    NetworkError(String),
    
    #[error("Parse error: {0}")]
    ParseError(String),
    
    #[error("Timeout error")]
    Timeout,
    
    #[error("Invalid URL: {0}")]
    InvalidUrl(String),
    
    #[error("FFI error: {0}")]
    FfiError(i32),
    
    #[error("IO error: {0}")]
    IoError(#[from] std::io::Error),
    
    #[error("JSON error: {0}")]
    JsonError(#[from] serde_json::Error),
    
    #[error("Unknown error: {0}")]
    Unknown(String),
}

pub type Result<T> = std::result::Result<T, BypsError>;

impl From<i32> for BypsError {
    fn from(code: i32) -> Self {
        match code {
            1 => BypsError::NetworkError("Network error".to_string()),
            2 => BypsError::ParseError("Parse error".to_string()),
            3 => BypsError::Timeout,
            4 => BypsError::InvalidUrl("Invalid URL".to_string()),
            _ => BypsError::Unknown(format!("Error code: {}", code)),
        }
    }
}
