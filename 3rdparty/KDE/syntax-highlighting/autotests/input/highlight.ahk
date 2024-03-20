^j:: ; hotkey label
::ftw::Free the whales ; hotstring label abbreviation
::btw:: ; hotstring label action
MsgBox, You typed btw.
Run, notepad.exe  ; Run Notepad when you press CTRL+N.
MsgBox, Wow!
MsgBox, There are
Run, notepad.exe
WinActivate, Untitled - Notepad
WinWaitActive, Untitled - Notepad
Send, 7 lines{!}{Enter}
SendInput, inside the CTRL{+}J hotkey.
return

Numpad0 & Numpad1::
MsgBox, You pressed Numpad1 while holding down Numpad0.
Run, notepad.exe
return

; Untitled - Notepad
#IfWinActive Untitled - Notepad
!q::
MsgBox, You pressed ALT+Q in Notepad.
return

; Any window that isn't Untitled - Notepad
#IfWinActive
!q::
MsgBox, You pressed ALT+Q in any window.
return

; Retrieve the ID/HWND of the active window
id := WinExist("A")
MsgBox % id

; Press Win+↑ to maximize the active window
#Up::WinMaximize, A

#i::
Run, https://www.google.com/
return

^p::
Run, notepad.exe
return

~j::
Send, ack
return

:*:acheiv::achiev
::achievment::achievement
::acquaintence::acquaintance
:*:adquir::acquir
::aquisition::acquisition
:*:agravat::aggravat
:*:allign::align
::ameria::America
:*:ftw::Free the whales ; Hotstring modifiers
this_is_a_label: ; label

#IfWinActive Untitled - Notepad
#Space::
MsgBox, You pressed WIN+SPACE in Notepad.
return

Send, {Ctrl down}c{Ctrl up}
SendInput, [b]{Ctrl down}v{Ctrl up}[/b]
return  ; This ends the hotkey. The code below this point will not get triggered.

Send, This text has been typed{!}
Send, {a}       ; WRONG
Send, {a}{b}{c} ; WRONG
Send, {abc}     ; WRONG
Send, abc       ; CORRECT
Send, ^s                     ; Both of these send CTRL+S
Send, {Ctrl down}s{Ctrl up}  ; Both of these send CTRL+S
Send, {Ctrl down}c{Ctrl up}
Send, {b down}{b up}
Send, {Tab down}{Tab up}
Send, {Up down}  ; Press down the up-arrow key.
Sleep, 1000      ; Keep it down for one second.
Send, {Up up}    ; Release the up-arrow key.

Send,
(
Line 1
Line 2
Apples are a fruit.
)

Send %A_Hour%
SubStr(37 * 12, 1, 2)
SubStr(A_Hour - 12, 2)
SubStr(A_AhkPath, InStr(A_AhkPath, "AutoHotkey"))
SubStr("I'm scripting, awesome!", 16)

SetTitleMatchMode RegEx
WinActivate ahk_exe i)\\notepad\.exe$  ; Match the name part of the full path.
WinActivate ahk_exe im)(*BSR_ANYCRLF)abc\Rxyz
WinActivate ahk_exe im)(*BSR_ANY[^]]CRLF)abc\Rxyz

if (MyVar = 5)
{
    MsgBox, MyVar equals %MyVar%!!
    ExitApp
}

MyVar = Text
MyVar = %MyVar2%
MyVar = %MyVar2% some text %MyVar3%.
MyVar := SubStr("I'm scripting, awesome!", 16)
MyVar := "Text"
MyVar := MyVar2
MyVar := 6 + 8 / 3 * 2 - Sqrt(9)
MyVar := "The value of 5 + " MyVar2 " is: " 5 + MyVar2
if (Var1 != Var2)
    Var1 := Var2 + 100

; Some examples showing when to use percents and when not:
Var = Text  ; Assign some text to a variable (legacy).
Number := 6  ; Assign a number to a variable (expression).
Var2 = %Var%  ; Assign a variable to another (legacy).
Var3 := Var  ; Assign a variable to another (expression).
Var4 .= Var  ; Append a variable to the end of another (expression).
Var5 += Var  ; Add the value of a variable to another (expression).
Var5 -= Var  ; Subtract the value of a variable from another (expression).
Var6 := SubStr(Var, 2, 2)  ; Variable inside a function. This is always an expression.
Var7 = %Var% Text  ; Assigns a variable to another with some extra text (legacy).
Var8 := Var " Text"  ; Assigns a variable to another with some extra text (expression).
MsgBox, %Var%  ; Variable inside a command.
StringSplit, Var, Var, x  ; Variable inside a command that uses InputVar and OutputVar.
if (Number = 6)  ; Whenever an IF has parentheses, it'll be an expression. So no percent signs.
if (Var != Number)  ; Whenever an IF has parentheses, it'll be an expression. So no percent signs.
if Number = 6  ; Without parentheses, the IF is legacy. However, only variables on the 'right side' need percent signs.
if Var1 < %Var2%  ; Without parentheses, the IF is legacy. However, only variables on the 'right side' need percent signs.

MyObject := ["one", "two", "three", 17]
Banana := {"Color": "Yellow", "Taste": "Delicious", "Price": 3}
MyObject := Array("one", "two", "three", 17)
Banana := Object("Color", "Yellow", "Taste", "Delicious", "Price", 3)
Banana["Pickled"] := True ; This banana has been pickled. Eww.
Banana.Consistency := "Mushy"
Value := Banana["Color"]
Value := Banana.Color
MyObject["NewerKey"] := 3.1415
MyObject.NewKey := "Shiny"
MyObject.Push(Value1, Value2, Value3...)
Banana.Consistency := ""
RemovedValue := MyObject.Delete(AnyKey)
NumberOfRemovedKeys := MyObject.Delete(FirstKey, LastKey)
arr := [{}]  ; Creates an array containing an object.
arr[1] := {}  ; Creates a second object, implicitly freeing the first object.
arr.RemoveAt(1)  ; Removes and frees the second object.
x := {}, y := {}             ; Create two objects.
x.child := y, y.parent := x  ; Create a circular reference.
y.parent := ""
x := "", y := ""
table.base.__Get(table, x)[y] := content   ; A
table.base.__Set(table, x, y, content)     ; B
RemovedValue := MyObject.RemoveAt(Index)
NumberOfRemovedKeys := MyObject.RemoveAt(Index, Length)
val := obj.Property := 42
m1 := new GMem(0, 20)
m2 := {base: GMem}.__New(0, 30)
x ? CallIfTrue() : CallIfFalse()
ProductIsAvailable := (Color = "Red")
    ? false  ; We don't have any red products, so don't bother calling the function.
    : ProductIsAvailableInColor(Product, Color)
MyObject.Pop()
%Var%()

Sleep MillisecondsToWait
Sleep %MillisecondsToWait%
Sleep % MillisecondsToWait
MsgBox % 1+1  ; Shows "2"
MsgBox   1+1  ; Shows "1+1"

MsgBox % "This is text."
MsgBox    This is text.
MsgBox %  A_AhkVersion
MsgBox   %A_AhkVersion%
MsgBox % %A_AhkVersion%
MsgBox % "Hello %A_UserName%."  ; Shows "%A_UserName%"
MsgBox    Hello %A_UserName%.   ; Shows your username.
MsgBox % "Hello " . A_UserName . "."  ; Shows your username.
MyVar := "This is text."
MyVar = This is text.

if (Var1 = Var2)
if Var1 = %Var2%
if (Var1 >= Low and Var1 <= High)
if Var1 between %Low% and %High%

Format("{:L}{:U}{:T}", input, input, input)

*#up::MouseMove, 0, -10, 0, R  ; Win+UpArrow hotkey => Move cursor upward
*#Down::MouseMove, 0, 10, 0, R  ; Win+DownArrow => Move cursor downward
*#Left::MouseMove, -10, 0, 0, R  ; Win+LeftArrow => Move cursor to the left
*#Right::MouseMove, 10, 0, 0, R  ; Win+RightArrow => Move cursor to the right

*<#RCtrl::  ; LeftWin + RightControl => Left-click (hold down Control/Shift to Control-Click or Shift-Click).
SendEvent {Blind}{LButton down}
KeyWait RCtrl  ; Prevents keyboard auto-repeat from repeating the mouse click.
SendEvent {Blind}{LButton up}
return

*<#AppsKey::  ; LeftWin + AppsKey => Right-click
SendEvent {Blind}{RButton down}
KeyWait AppsKey  ; Prevents keyboard auto-repeat from repeating the mouse click.
SendEvent {Blind}{RButton up}
return

#Persistent  ; Keep this script running until the user explicitly exits it.
SetTimer, WatchPOV, 5
return

WatchPOV:
POV := GetKeyState("JoyPOV")  ; Get position of the POV control.
KeyToHoldDownPrev := KeyToHoldDown  ; Prev now holds the key that was down before (if any).

; Some joysticks might have a smooth/continous POV rather than one in fixed increments.
; To support them all, use a range:
if (POV < 0)   ; No angle to report
    KeyToHoldDown := ""
else if (POV > 31500)               ; 315 to 360 degrees: Forward
    KeyToHoldDown := "Up"
else if POV between 0 and 4500      ; 0 to 45 degrees: Forward
    KeyToHoldDown := "Up"
else if POV between 4501 and 13500  ; 45 to 135 degrees: Right
    KeyToHoldDown := "Right"
else if POV between 13501 and 22500 ; 135 to 225 degrees: Down
    KeyToHoldDown := "Down"
else                                ; 225 to 315 degrees: Left
    KeyToHoldDown := "Left"

if (KeyToHoldDown = KeyToHoldDownPrev)  ; The correct key is already down (or no key is needed).
    return  ; Do nothing.

; Otherwise, release the previous key and press down the new key:
SetKeyDelay -1  ; Avoid delays between keystrokes.
if KeyToHoldDownPrev   ; There is a previous key to release.
    Send, {%KeyToHoldDownPrev% up}  ; Release it.
if KeyToHoldDown   ; There is a key to press down.
    Send, {%KeyToHoldDown% down}  ; Press it down.
return

<^>!m::MsgBox You pressed AltGr+m.
<^<!m::MsgBox You pressed LeftControl+LeftAlt+m.

AppsKey::ToolTip Press < or > to cycle through windows.
AppsKey Up::ToolTip
~AppsKey & <::Send !+{Esc}
~AppsKey & >::Send !{Esc}

; Press AppsKey and Alt in any order, then slash (/).
#if GetKeyState("AppsKey", "P")
Alt & /::MsgBox Hotkey activated.

; If the keys are swapped, Alt must be pressed first (use one at a time):
#if GetKeyState("Alt", "P")
AppsKey & /::MsgBox Hotkey activated.

; [ & ] & \::
#if GetKeyState("[") && GetKeyState("]")
\::MsgBox

; Ctrl+Shift+O to open containing folder in Explorer.
; Ctrl+Shift+E to open folder with current file selected.
; Supports SciTE and Notepad++.
^+o::
^+e::
    editor_open_folder() {
        WinGetTitle, path, A
        if RegExMatch(path, "\*?\K(.*)\\[^\\]+(?= [-*] )", path)
            if (FileExist(path) && A_ThisHotkey = "^+e")
                Run explorer.exe /select`,"%path%"
            else
                Run explorer.exe "%path1%"
    }

#h::  ; Win+H hotkey
; Get the text currently selected. The clipboard is used instead of
; "ControlGet Selected" because it works in a greater variety of editors
; (namely word processors).  Save the current clipboard contents to be
; restored later. Although this handles only plain text, it seems better
; than nothing:
AutoTrim Off  ; Retain any leading and trailing whitespace on the clipboard.
ClipboardOld := ClipboardAll
Clipboard := ""  ; Must start off blank for detection to work.
Send ^c
ClipWait 1
if ErrorLevel  ; ClipWait timed out.
    return
; Replace CRLF and/or LF with `n for use in a "send-raw" hotstring:
; The same is done for any other characters that might otherwise
; be a problem in raw mode:
StringReplace, Hotstring, Clipboard, ``, ````, All  ; Do this replacement first to avoid interfering with the others below.
StringReplace, Hotstring, Hotstring, `r`n, ``r, All  ; Using `r works better than `n in MS Word, etc.
StringReplace, Hotstring, Hotstring, `n, ``r, All
StringReplace, Hotstring, Hotstring, %A_Tab%, ``t, All
StringReplace, Hotstring, Hotstring, `;, ```;, All
Clipboard := ClipboardOld  ; Restore previous contents of clipboard.
; This will move the InputBox's caret to a more friendly position:
SetTimer, MoveCaret, 10
if ErrorLevel  ; The user pressed Cancel.
    return
if InStr(Hotstring, ":R`:::")
{
    MsgBox You didn't provide an abbreviation. The hotstring has not been added.
    return
}
; Otherwise, add the hotstring and reload the script:
FileAppend, `n%Hotstring%, %A_ScriptFullPath%  ; Put a `n at the beginning in case file lacks a blank line at its end.
Reload
Sleep 200
MsgBox, 4,, The hotstring just added appears to be improperly formatted.
IfMsgBox, Yes, Edit
return

MoveCaret:
IfWinNotActive, New Hotstring
    return
; Otherwise, move the InputBox's insertion point to where the user will type the abbreviation.
Send {Home}{Right 3}
SetTimer, MoveCaret, Off
return

; This example also demonstrates one way to implement case conformity in a script.
:C:BTW::  ; Typed in all-caps.
:C:Btw::  ; Typed with only the first letter upper-case.
: :btw::  ; Typed in any other combination.
    case_conform_btw() {
        hs := A_ThisHotkey  ; For convenience and in case we're interrupted.
        if (hs == ":C:BTW")
            Send BY THE WAY
        else if (hs == ":C:Btw")
            Send By the way
        else
            Send by the way
    }

#IfWinActive ahk_class Notepad
::btw::This replacement text will appear only in Notepad.
#IfWinActive
::btw::This replacement text appears in windows other than Notepad.

#Hotstring EndChars -()[]{}:;'"/\,.?!`n `t

Hotstring("EndChars", "-()[]{}:;")

::btw::
MsgBox You typed "``btw``".
return

:*:]d::  ; This hotstring replaces "]d" with the current date and time via the commands below.

MyFunction(FirstParameter, Second, ByRef Third, Fourth:="")
{
    return "a value"
}

Loop 3
    MsgBox % MyArray%A_Index%

SysGet, WA, MonitorWorkArea
MsgBox, Left: %WALeft% -- Top: %WATop% -- Right: %WARight% -- Bottom: %WABottom%.

n := 123 00123 -1.  0x7B 0x007B -0x1  3.14159

FileAppend,   ; Comment.
; Comment.
( LTrim Join    ; Comment.
     ; This is not a comment; it is literal. Include the word Comments in the line above to make it a comment.
), C:\File.txt   ; Comment.

param := %A_Index%  ; Fetch the contents of the variable whose name is contained in A_Index.

Add(X, Y, Z:=0) {
    return X + Y + Z
}

Join(sep, params*) {
    for index,param in params
        str .= param . sep
    return SubStr(str, 1, -StrLen(sep))
}
MsgBox % Join("`n", "one", "two", "three")

LogToFile(TextToLog)
{
    global LogFileName  ; This global variable was previously given a value somewhere outside this function.
    FileAppend, %TextToLog%`n, %LogFileName%
}

SetDefaults()
{
    global
    MyGlobal := 33
    local x, y:=0, z
}

LogToFile(TextToLog)
{
    static LoggedLines := 0
    LoggedLines += 1
    global LogFileName
    FileAppend, %LoggedLines%: %TextToLog%`n, %LogFileName%
}

GetFromStaticArray(WhichItemNumber)
{
    static
    static FirstCallToUs := true
    if FirstCallToUs
    {
        FirstCallToUs := false
        Loop 10
            StaticArray%A_Index% := "Value #" . A_Index
    }
    return StaticArray%WhichItemNumber%
}

if (ColorName != "" AND not FindColor(ColorName))
    MsgBox %ColorName% could not be found.

class baseObject {
    static foo := "bar"
}
baseObject := {foo: "bar"}

thing := {}
thing.foo := "bar"
thing.test := Func("thing_test")
thing.test()

thing_test(this) {
    MsgBox % this.foo
}

class Color
{
    __New(aRGB)
    {
        this.RGB := aRGB
    }

    __Delete()
    {
        MsgBox % "Delete Color."
    }

    static Shift := {R:16, G:8, B:0}

    __Get(aName)
    {
        ; NOTE: Using this.Shift here would cause an infinite loop!
        shift := Color.Shift[aName]  ; Get the number of bits to shift.
        if (shift != "")  ; Is it a known property?
            return (this.RGB >> shift) & 0xff
        ; NOTE: Using 'return' here would break this.RGB.
    }

    __Set(aName, aValue)
    {
        if ((shift := Color.Shift[aName]) != "")
        {
            aValue &= 255  ; Truncate it to the proper range.

            ; Calculate and store the new RGB value.
            this.RGB := (aValue << shift) | (this.RGB & ~(0xff << shift))

            ; 'Return' must be used to indicate a new key-value pair should not be created.
            ; This also defines what will be stored in the 'x' in 'x := clr[name] := val':
            return aValue
        }
        ; NOTE: Using 'return' here would break this.stored_RGB and this.RGB.
    }

    ; Meta-functions can be mixed with properties:
    RGB {
        get {
            ; Return it in hex format:
            return format("0x{:06x}", this.stored_RGB)
        }
        set {
            return this.stored_RGB := value
        }
    }

    class __Get extends Properties
    {
        R() {
            return (this.RGB >> 16) & 255
        }
        G() {
            return (this.RGB >> 8) & 255
        }
        B() {
            return this.RGB & 255
        }
    }

    Property[]  ; Brackets are optional
    {
        get {
            return ...
        }
        set {
            return ... := value
        }
    }
}

class Properties extends FunctionObject
{
    Call(aTarget, aName, aParams*)
    {
        ; If this Properties object contains a definition for this half-property, call it.
        if ObjHasKey(this, aName)
            return this[aName].Call(aTarget, aParams*)
    }
}

MyGet(this, Key, Key2)
MySet(this, Key, Key2, Value)
MyCall(this, Name, Params)

ClassName := { __Get: Func("MyGet"), __Set: Func("MySet"), __Call: Func("MyCall") }

red  := new Color(0xff0000), red.R -= 5
cyan := new Color(0), cyan.G := 255, cyan.B := 255

MsgBox % "red: " red.R "," red.G "," red.B " = " red.RGB
MsgBox % "cyan: " cyan.R "," cyan.G "," cyan.B " = " cyan.RGB

; This example requires the FunctionObject class in order to work.
blue := new Color(0x0000ff)
MsgBox % blue.R "," blue.G "," blue.B

x := {base: {addr: Func("x_Addr"), __Set: Func("x_Setter")}}

; Assign value, implicitly calling x_Setter to create sub-objects.
x[1,2,3] := "..."

; Retrieve value and call example method.
MsgBox % x[1,2,3] "`n" x.addr() "`n" x[1].addr() "`n" x[1,2].addr()

x_Setter(x, p1, p2, p3) {
    x[p1] := new x.base
}

x_Addr(x) {
    return &x
}

InputBox, OutputVar, Question 1, What is your first name?
if (OutputVar = "Bill")
    MsgBox, That's an awesome name`, %OutputVar%.

InputBox, OutputVar2, Question 2, Do you like AutoHotkey?
if (OutputVar2 = "yes")
    MsgBox, Thank you for answering %OutputVar2%`, %OutputVar%! We will become great friends.
else
    MsgBox, %OutputVar%`, That makes me sad.

MsgBox, 4,, Would you like to continue?
IfMsgBox, No
    return  ; If No, stop the code from going further.
MsgBox, You pressed YES.  ; Otherwise, the user picked yes.

if (car = "old")
{
    MsgBox, The car is really old.
    if (wheels = "flat")
    {
        MsgBox, This car is not safe to drive.
        return
    }
    else
    {
        MsgBox, Be careful! This old car will be dangerous to drive.
    }
}
else
{
    MsgBox, My`, what a shiny new vehicle you have there.
}

if (Color = "Red" or Color = "Green"  or Color = "Blue"   ; Comment.
    or Color = "Black" or Color = "Gray" or Color = "White")   ; Comment.
    and ProductIsAvailableInColor(Product, Color)   ; Comment.

if (codepage != "")
    codepage := " /CP" . codepage
cmd="%A_AhkPath%"%codepage% "`%1" `%*
key=AutoHotkeyScript\Shell\Open\Command
if A_IsAdmin    ; Set for all users.
    RegWrite, REG_SZ, HKCR, %key%,, %cmd%
else            ; Set for current user only.
    RegWrite, REG_SZ, HKCU, Software\Classes\%key%,, %cmd%

^j:: ; hotkey label
MsgBox, You typed btw.
