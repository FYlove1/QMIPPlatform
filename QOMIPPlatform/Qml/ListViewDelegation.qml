import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    width: 600
    height: 150

    // 主布局容器 - 使用RowLayout替代Row以更好地控制弹性空间
    RowLayout {
        anchors.fill: parent
        anchors.margins: root.height * 0.10
        spacing: root.width * 0.03  // 列间距设为相对值，适应不同屏幕宽度

        // 第1列：Name / Text
        ColumnLayout {
            Layout.alignment: Qt.AlignTop
            spacing: root.height * 0.08

            Label {
                text: qsTr("Name")
                font.pixelSize: Math.max(12, root.height * 0.18)
                Layout.alignment: Qt.AlignLeft
            }

            Label {
                text: qsTr("Text")
                font.pixelSize: Math.max(12, root.height * 0.18)
                Layout.alignment: Qt.AlignLeft
            }
        }

        // 第2列：parameter / ComboBox
        ColumnLayout {
            Layout.alignment: Qt.AlignTop
            spacing: root.height * 0.08

            Label {
                text: qsTr("parameter")
                font.pixelSize: Math.max(12, root.height * 0.18)
                Layout.alignment: Qt.AlignLeft
            }

            ComboBox {
                id: paramComboBox
                Layout.preferredWidth: Math.max(120, root.width * 0.22)
                Layout.preferredHeight: Math.max(30, root.height * 0.22)
                model: ["A", "B", "C"]
            }
        }

        // 第3列：value / TextEdit
        ColumnLayout {
            Layout.alignment: Qt.AlignTop
            spacing: root.height * 0.08

            Label {
                text: qsTr("value")
                font.pixelSize: Math.max(12, root.height * 0.18)
                Layout.alignment: Qt.AlignLeft
            }

            Rectangle {
                Layout.preferredWidth: Math.max(120, root.width * 0.22)
                Layout.preferredHeight: Math.max(30, root.height * 0.22)
                border.width: 1
                border.color: "#CCCCCC"
                radius: 4

                TextEdit {
                    anchors.fill: parent
                    anchors.margins: 4
                    font.pixelSize: Math.max(12, root.height * 0.18)
                    text: ""
                    verticalAlignment: TextEdit.AlignVCenter
                }
            }
        }

        // 删除按钮部分
        ColumnLayout {
            Layout.alignment: Qt.AlignBottom | Qt.AlignRight
            Layout.rightMargin: root.width * 0.02

            RoundButton {
                text: qsTr("Delete")
                font.pixelSize: Math.max(10, root.height * 0.15)
                Layout.preferredWidth: Math.max(80, root.width * 0.15)
                Layout.preferredHeight: Math.max(30, root.height * 0.22)

                background: Rectangle {
                    radius: height / 2
                    color: parent.pressed ? "#AA4444" : "#CC5555"
                    border.color: "#993333"
                    border.width: 1
                }

                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        // 填充剩余空间
        Item {
            Layout.fillWidth: true
        }
    }
}
