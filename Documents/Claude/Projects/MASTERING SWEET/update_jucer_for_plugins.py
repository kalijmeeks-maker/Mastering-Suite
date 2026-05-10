#!/usr/bin/env python3
"""
Update .jucer file to add VST3 and AU plugin export targets
"""
import xml.etree.ElementTree as ET

jucer_path = "MasteringSuite/MasteringSuite.jucer"

# Parse the existing .jucer file
tree = ET.parse(jucer_path)
root = tree.getroot()

# Find the EXPORTFORMATS section
export_formats = root.find(".//EXPORTFORMATS")
if export_formats is None:
    print("ERROR: EXPORTFORMATS not found in .jucer file")
    exit(1)

# Get the first XCODE_MAC element to use as a template
first_xcode = export_formats.find("XCODE_MAC")
if first_xcode is None:
    print("ERROR: No XCODE_MAC export found")
    exit(1)

# Create VST3 export target by copying and modifying the first one
vst3_export = ET.Element("XCODE_MAC")
vst3_export.set("targetFolder", "Builds/MacOSX")
vst3_export.set("smallIcon", "0")
vst3_export.set("bigIcon", "0")
vst3_export.set("microphonePermissionNeeded", "0")
vst3_export.set("cameraPermissionNeeded", "0")

# Copy configurations
configs_vst3 = ET.SubElement(vst3_export, "CONFIGURATIONS")
for config in first_xcode.findall("CONFIGURATIONS/CONFIGURATION"):
    new_config = ET.Element("CONFIGURATION")
    new_config.set("isDebug", config.get("isDebug"))
    new_config.set("name", config.get("name"))
    new_config.set("targetName", f"MasteringSuite-VST3{'-Debug' if config.get('isDebug') == '1' else ''}")
    if config.get("optimisation"):
        new_config.set("optimisation", config.get("optimisation"))
    configs_vst3.append(new_config)

# Copy module paths
modulepaths_vst3 = ET.SubElement(vst3_export, "MODULEPATHS")
for modulepath in first_xcode.findall("MODULEPATHS/MODULEPATH"):
    new_modulepath = ET.Element("MODULEPATH")
    new_modulepath.set("id", modulepath.get("id"))
    new_modulepath.set("path", modulepath.get("path"))
    modulepaths_vst3.append(new_modulepath)

# Add VST3 export to EXPORTFORMATS
export_formats.append(vst3_export)

# Create AU export target
au_export = ET.Element("XCODE_MAC")
au_export.set("targetFolder", "Builds/MacOSX")
au_export.set("smallIcon", "0")
au_export.set("bigIcon", "0")
au_export.set("microphonePermissionNeeded", "0")
au_export.set("cameraPermissionNeeded", "0")

# Copy configurations for AU
configs_au = ET.SubElement(au_export, "CONFIGURATIONS")
for config in first_xcode.findall("CONFIGURATIONS/CONFIGURATION"):
    new_config = ET.Element("CONFIGURATION")
    new_config.set("isDebug", config.get("isDebug"))
    new_config.set("name", config.get("name"))
    new_config.set("targetName", f"MasteringSuite-AU{'-Debug' if config.get('isDebug') == '1' else ''}")
    if config.get("optimisation"):
        new_config.set("optimisation", config.get("optimisation"))
    configs_au.append(new_config)

# Copy module paths for AU
modulepaths_au = ET.SubElement(au_export, "MODULEPATHS")
for modulepath in first_xcode.findall("MODULEPATHS/MODULEPATH"):
    new_modulepath = ET.Element("MODULEPATH")
    new_modulepath.set("id", modulepath.get("id"))
    new_modulepath.set("path", modulepath.get("path"))
    modulepaths_au.append(new_modulepath)

# Add AU export to EXPORTFORMATS
export_formats.append(au_export)

# Write the updated .jucer file
tree.write(jucer_path, encoding='utf-8', xml_declaration=True)
print("✅ Updated .jucer file with VST3 and AU export targets")
print("   - Added VST3 plugin export")
print("   - Added AU plugin export")
print("   Next: Run Projucer to regenerate Xcode project")
