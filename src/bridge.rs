use std::ffi::{CStr, CString};
use std::os::raw::c_char;
use crate::error::{BypsError, Result};

// FFI declarations
#[repr(C)]
pub struct BypsEngine {
    _private: [u8; 0],
}

extern "C" {
    pub fn byps_engine_new() -> *mut BypsEngine;
    pub fn byps_engine_free(engine: *mut BypsEngine);
    pub fn byps_engine_scan(
        engine: *mut BypsEngine,
        url: *const c_char,
        config_json: *const c_char,
        result_json: *mut *mut c_char,
    ) -> i32;
    pub fn byps_engine_test_technique(
        engine: *mut BypsEngine,
        url: *const c_char,
        technique_name: *const c_char,
        result_json: *mut *mut c_char,
    ) -> i32;
    pub fn byps_engine_test_variations(
        engine: *mut BypsEngine,
        base_url: *const c_char,
        config_json: *const c_char,
        result_json: *mut *mut c_char,
    ) -> i32;
    pub fn byps_engine_detect_waf(
        engine: *mut BypsEngine,
        url: *const c_char,
        waf_info_json: *mut *mut c_char,
    ) -> i32;
    pub fn byps_string_free(s: *mut c_char);
    pub fn byps_version() -> *const c_char;
    pub fn byps_last_error(engine: *mut BypsEngine) -> *const c_char;
}

pub struct Engine {
    ptr: *mut BypsEngine,
}

impl Engine {
    pub fn new() -> Result<Self> {
        let ptr = unsafe { byps_engine_new() };
        if ptr.is_null() {
            return Err(BypsError::Unknown("Failed to create engine".to_string()));
        }
        Ok(Engine { ptr })
    }
    
    pub fn scan(&self, url: &str, config: &str) -> Result<String> {
        let url_c = CString::new(url).map_err(|_| BypsError::InvalidUrl(url.to_string()))?;
        let config_c = CString::new(config).unwrap_or_else(|_| CString::new("{}").unwrap());
        
        let mut result_ptr: *mut c_char = std::ptr::null_mut();
        
        let error_code = unsafe {
            byps_engine_scan(
                self.ptr,
                url_c.as_ptr(),
                config_c.as_ptr(),
                &mut result_ptr as *mut *mut c_char,
            )
        };
        
        if error_code != 0 {
            return Err(BypsError::from(error_code));
        }
        
        if result_ptr.is_null() {
            return Err(BypsError::Unknown("Null result".to_string()));
        }
        
        let result = unsafe {
            let c_str = CStr::from_ptr(result_ptr);
            let rust_str = c_str.to_string_lossy().into_owned();
            byps_string_free(result_ptr);
            rust_str
        };
        
        Ok(result)
    }
    
    pub fn test_technique(&self, url: &str, technique: &str) -> Result<String> {
        let url_c = CString::new(url).map_err(|_| BypsError::InvalidUrl(url.to_string()))?;
        let technique_c = CString::new(technique).unwrap();
        
        let mut result_ptr: *mut c_char = std::ptr::null_mut();
        
        let error_code = unsafe {
            byps_engine_test_technique(
                self.ptr,
                url_c.as_ptr(),
                technique_c.as_ptr(),
                &mut result_ptr as *mut *mut c_char,
            )
        };
        
        if error_code != 0 {
            return Err(BypsError::from(error_code));
        }
        
        if result_ptr.is_null() {
            return Err(BypsError::Unknown("Null result".to_string()));
        }
        
        let result = unsafe {
            let c_str = CStr::from_ptr(result_ptr);
            let rust_str = c_str.to_string_lossy().into_owned();
            byps_string_free(result_ptr);
            rust_str
        };
        
        Ok(result)
    }
    
    pub fn test_variations(&self, url: &str, config: &str) -> Result<String> {
        let url_c = CString::new(url).map_err(|_| BypsError::InvalidUrl(url.to_string()))?;
        let config_c = CString::new(config).unwrap_or_else(|_| CString::new("{}").unwrap());
        
        let mut result_ptr: *mut c_char = std::ptr::null_mut();
        
        let error_code = unsafe {
            byps_engine_test_variations(
                self.ptr,
                url_c.as_ptr(),
                config_c.as_ptr(),
                &mut result_ptr as *mut *mut c_char,
            )
        };
        
        if error_code != 0 {
            return Err(BypsError::from(error_code));
        }
        
        if result_ptr.is_null() {
            return Err(BypsError::Unknown("Null result".to_string()));
        }
        
        let result = unsafe {
            let c_str = CStr::from_ptr(result_ptr);
            let rust_str = c_str.to_string_lossy().into_owned();
            byps_string_free(result_ptr);
            rust_str
        };
        
        Ok(result)
    }
    
    pub fn version() -> String {
        unsafe {
            let c_str = CStr::from_ptr(byps_version());
            c_str.to_string_lossy().into_owned()
        }
    }
}

impl Drop for Engine {
    fn drop(&mut self) {
        unsafe {
            byps_engine_free(self.ptr);
        }
    }
}

unsafe impl Send for Engine {}
unsafe impl Sync for Engine {}
