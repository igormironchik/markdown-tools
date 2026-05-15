import QtQuick
import QtQuick.Window
import org.kde.kirigami as Kirigami
import QtTest

TestCase {
    id: testCase

    Kirigami.MnemonicData.enabled: true
    Kirigami.MnemonicData.label: "设置(&S)"

    width: 400
    height: 400

    SignalSpy {
        id: sequenceChangedSpy
        target: testCase.Kirigami.MnemonicData
        signalName: "sequenceChanged"
    }

    function test_press() {
        compare(Kirigami.MnemonicData.richTextLabel, "设置")
    }

    function test_disable_shortcut() {
        compare(sequenceChangedSpy.count, 0);
        testCase.Kirigami.MnemonicData.enabled = false;
        compare(sequenceChangedSpy.count, 1);
        testCase.Kirigami.MnemonicData.enabled = true;
        compare(sequenceChangedSpy.count, 2);
    }
}
