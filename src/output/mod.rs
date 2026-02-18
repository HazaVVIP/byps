pub mod json;
pub mod terminal;

use crate::error::Result;

pub trait OutputFormatter {
    fn format(&self, data: &str) -> Result<String>;
    fn write_to_file(&self, data: &str, path: &str) -> Result<()>;
}
