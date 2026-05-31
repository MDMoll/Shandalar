Set shell = CreateObject("WScript.Shell")

For Each shortcutPath In WScript.Arguments
  Set shortcut = shell.CreateShortcut(shortcutPath)
  WScript.Echo "Shortcut: " & shortcutPath
  WScript.Echo "TargetPath: " & shortcut.TargetPath
  WScript.Echo "Arguments: " & shortcut.Arguments
  WScript.Echo "WorkingDirectory: " & shortcut.WorkingDirectory
  WScript.Echo ""
Next
