#![cfg(feature = "serialize")]

use std::path::PathBuf;

use fontconfig_parser::*;

fn test_single_conf(path: PathBuf) -> Result<()> {
    eprintln!("Test {}", path.display());

    let json_path = path.parent().unwrap().join(format!(
        "{}.json",
        path.file_name().unwrap().to_str().unwrap()
    ));

    let parts = parse_config_parts(std::fs::read_to_string(path)?.as_str())?;

    let expected = std::fs::read_to_string(json_path)?;
    let actual = serde_json::to_string(&parts).unwrap();
    k9::assert_equal!(expected, actual);

    Ok(())
}

#[test]
fn test_conf() -> Result<()> {
    test_single_conf("./test-conf/fonts.conf".into())?;

    for conf in glob::glob("./test-conf/conf.d/*.conf").unwrap() {
        test_single_conf(conf.unwrap())?;
    }

    Ok(())
}
