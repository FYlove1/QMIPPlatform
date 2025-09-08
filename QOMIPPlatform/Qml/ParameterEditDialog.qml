import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Dialog {
    id: root
    title: algorithmName ? "编辑参数: " + algorithmName : "编辑算法参数"
    modal: true
    width: 400
    height: 450
    
    // 属性定义
    property int algorithmIndex: -1
    property int algorithmId: -1
    property var params: ({})
    property string algorithmDescription: ""
    property string algorithmName: ""
    
    // 信号定义
    signal parametersAccepted(int index, var parameters)
    
    // 打开对话框并接收参数
    function openDialog(index, id, paramsData, metaData, description, name) {
        try {
            // 清空之前的控件
            paramContainer.children = []
            
            // 设置所有属性
            algorithmIndex = index
            algorithmId = id
            
            // 创建参数的深拷贝，避免直接修改原始数据
            params = JSON.parse(JSON.stringify(paramsData || {}))
            algorithmDescription = description || ""
            algorithmName = name || ""

            console.log("获取到参数:", JSON.stringify(params))
            console.log("获取到描述:", algorithmDescription)
            console.log("描述是否为空:", !algorithmDescription)
            console.log("描述长度:", algorithmDescription ? algorithmDescription.length : 0)

            // 重建参数UI
            refreshParameterUI()

            // 打开对话框
            open()
        } catch (e) {
            console.error("打开参数对话框时发生错误:", e)
        }
    }
    
    // 刷新参数UI函数
    function refreshParameterUI() {
        try {
            // 清空之前的控件
            while (paramContainer.children.length > 0) {
                paramContainer.children[0].destroy()
            }

            // 添加算法描述
            if (algorithmDescription) {
                let descLoader = Qt.createComponent("DescriptionComponent.qml")
                if (descLoader.status === Component.Ready) {
                    let descriptionItem = descLoader.createObject(paramContainer, {
                                                                      "description": algorithmDescription
                                                                  })
                } else {
                    console.error("加载描述组件失败:", descLoader.errorString())
                }
            }

            // 创建参数编辑界面
            createParameterUI()
        } catch (e) {
            console.error("刷新参数UI时发生错误:", e)
        }
    }
    
    // 创建参数UI
    function createParameterUI() {
        try {
            // 检查是否有参数
            if (Object.keys(params).length === 0) {
                let noParamsLoader = Qt.createComponent("NoParamsComponent.qml")
                if (noParamsLoader.status === Component.Ready) {
                    noParamsLoader.createObject(paramContainer)
                } else {
                    console.error("加载无参数组件失败:", noParamsLoader.errorString())
                }
                return
            }

            // 为每个参数创建一个编辑字段
            for (let key in params) {
                let value = params[key]
                let type = typeof value
                
                let paramItem
                
                // 根据值类型加载不同的组件
                if (type === "number") {
                    let loader = Qt.createComponent("NumberInputComponent.qml")
                    if (loader.status === Component.Ready) {
                        paramItem = loader.createObject(paramContainer, {
                                                            "paramName": key,
                                                            "displayName": key,
                                                            "paramValue": value,
                                                            "isDouble": !Number.isInteger(value),
                                                            "dialogRef": root  // 传递对话框引用
                                                        })
                    } else {
                        console.error("加载数字输入组件失败:", loader.errorString())
                    }
                } else if (type === "boolean") {
                    let loader = Qt.createComponent("BooleanInputComponent.qml")
                    if (loader.status === Component.Ready) {
                        paramItem = loader.createObject(paramContainer, {
                                                            "paramName": key,
                                                            "displayName": key,
                                                            "paramValue": value,
                                                            "dialogRef": root  // 传递对话框引用
                                                        })
                    } else {
                        console.error("加载布尔输入组件失败:", loader.errorString())
                    }
                } else {
                    let loader = Qt.createComponent("TextInputComponent.qml")
                    if (loader.status === Component.Ready) {
                        paramItem = loader.createObject(paramContainer, {
                                                            "paramName": key,
                                                            "displayName": key,
                                                            "paramValue": value,
                                                            "dialogRef": root  // 传递对话框引用
                                                        })
                    } else {
                        console.error("加载文本输入组件失败:", loader.errorString())
                    }
                }
                
                // 添加分隔线
                if (paramItem) {
                    let separatorRect = Qt.createQmlObject(`
                                                           import QtQuick 2.15
                                                           Rectangle {
                                                           width: parent ? parent.width : 350
                                                           height: 1
                                                           color: "#dddddd"
                                                           }
                                                           `, paramContainer)
                }
            }
        } catch (e) {
            console.error("创建参数UI时发生错误:", e)
        }
    }
    
    contentItem: Item {
    // 不要使用fill占满整个区域
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    // 留出底部空间
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 60 // 为底部按钮预留空间
    
    ScrollView {
        id: scrollView
        anchors.fill: parent
        contentWidth: width
        clip: true
        
        // 设置滚动条策略
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        
        // 参数容器
        Column {
            id: paramContainer
            width: scrollView.width - 20 // 减去滚动条宽度
            spacing: 10
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 10
        }
    }
}
    
    // 加强底部按钮区域样式，使其更加明显
footer: Rectangle {
    id: footerRect
    width: parent.width
    height: 60 // 增加高度
    color: "#e0e0e0" // 更明显的背景色
    
    // 添加顶部边框线
    Rectangle {
        width: parent.width
        height: 1
        color: "#cccccc"
        anchors.top: parent.top
    }

    // 使用RowLayout放置按钮
    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Item { // 左侧弹性空间
            Layout.fillWidth: true
        }

        // 取消按钮
        Button {
            text: "取消"
            Layout.preferredWidth: 80
            Layout.preferredHeight: 36 // 增加按钮高度
            onClicked: {
                console.log("取消按钮被点击")
                root.reject() // 直接调用Dialog的reject方法
            }
        }

        // 确认按钮
        Button {
            text: "确认"
            Layout.preferredWidth: 80
            Layout.preferredHeight: 36 // 增加按钮高度
            highlighted: true // 高亮显示
            onClicked: {
                console.log("确认按钮被点击")
                root.accept() // 直接调用Dialog的accept方法，会触发onAccepted
            }
        }
    }
}
    
    // 处理对话框结果
onAccepted: {
    try {
        console.log("============ [QML-ParameterEditDialog] 参数更新流程开始 ============")
        console.log("[QML-STEP1] 算法索引:", algorithmIndex)
        console.log("[QML-STEP2] 即将更新的参数:", JSON.stringify(params))
        
        // 输出每个参数的详细信息
        for (let key in params) {
            console.log("[QML-PARAM]", key, ":", params[key], "(类型:", typeof params[key], ")")
        }

        // 调用C++方法更新参数
        console.log("[QML-STEP3] 调用 algorithmModel.updateAlgorithmParameters...")
        const success = algorithmModel.updateAlgorithmParameters(algorithmIndex, params)
        console.log("[QML-STEP4] C++返回结果:", success)
        
        if (success) {
            // 使用Timer延迟发送信号，避免在锁定状态下触发信号处理
            Qt.callLater(function() {
                parametersAccepted(algorithmIndex, params)
                console.log("[QML-STEP5] 信号已发送: parametersAccepted")
            })
        } else {
            console.error("[QML-ERROR] 更新参数失败")
        }
        console.log("============ [QML-ParameterEditDialog] 参数更新流程结束 ============")
    } catch (e) {
        console.error("[QML-ERROR] 保存参数时发生异常:", e)
    }
}
}
