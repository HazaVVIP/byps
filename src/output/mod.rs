pub mod json;
pub mod terminal;
pub mod csv;
pub mod html;

use crate::error::Result;

pub trait OutputFormatter {
    fn format(&self, data: &str) -> Result<String>;
    fn write_to_file(&self, data: &str, path: &str) -> Result<()>;
}
