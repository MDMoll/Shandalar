Set shell = CreateObject("WScript.Shell")

Sub FocusShandalar()
  Dim i
  For i = 1 To 20
    If shell.AppActivate("Magic: Shandalar") Then Exit Sub
    If shell.AppActivate("Shandalar.exe") Then Exit Sub
    If shell.AppActivate("Shandalar") Then Exit Sub
    WScript.Sleep 250
  Next
End Sub

WScript.Sleep 3000
FocusShandalar
WScript.Sleep 500
shell.SendKeys "~"

WScript.Sleep 1500
FocusShandalar
WScript.Sleep 250
shell.SendKeys "~"

WScript.Sleep 1500
FocusShandalar
WScript.Sleep 250
shell.SendKeys "~"

WScript.Sleep 8000
