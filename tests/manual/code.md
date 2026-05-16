# Java and C++ examples

## Java

```java
import java.util.Scanner;

public class HelloWorld {

    public static void main(String[] args) {

        // Creates a reader instance which takes
        // input from standard input - keyboard
        Scanner reader = new Scanner(System.in);
        System.out.print("Enter a number: ");

        // nextInt() reads the next integer from the keyboard
        int number = reader.nextInt();

        // println() prints the following line to the output screen
        System.out.println("You entered: " + number);
    }
}
```

## C++

```cpp
#include <iostream>
using namespace std;

int main()
{    
    int number;

    cout << "Enter an integer: ";
    cin >> number;

    cout << "You entered " << number;    
    return 0;
}
```

## QML

```qml
import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Window 2.15

Dialog {
    property size appWindowSize;
    property string turnText;
    property real radioSize: Screen.pixelDensity * 4

    title: qsTr( "Checkmate..." )
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    width: appWindowSize.width
    height: Math.max( appWindowSize.height / 2, column.height + implicitHeaderHeight + 10 )
    x: 0
    y: appWindowSize.height / 2 - height / 2

    Column {
        anchors.centerIn: parent
        id: column

        Text {
            font.pointSize: 30
            font.bold: true
            text: ( turnText === qsTr( "Black" ) ?
                qsTr( "White" ) : qsTr( "Black" ) ) + qsTr( " won!!!" )
        }

        Rectangle {
            height: 25
            width: column.width
        }

        Button {
            text: qsTr( "OK" )
            anchors.right: column.right
            implicitHeight: radioSize * 2.5
            implicitWidth: radioSize * 5

            onClicked: {
                close()
            }
        }
    }
}
```
