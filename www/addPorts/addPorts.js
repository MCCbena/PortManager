let is_change = false; //ポートが編集か新規作成かのどちからが入力される
//要素を取得
const modal = document.querySelector('.js-modal'),
    open = document.querySelector('#add-setting-btn'),
    close = document.querySelector('.js-modal-close');
const forms = [
    document.getElementById("port-name-field"),
    document.getElementById("ipaddress-field"),
    document.getElementById("port-field"),
    document.getElementById("protocol-field")
]
//「追加」をクリックしてモーダルを開く
function modalOpen() {
    if(current_server!=="") {
        is_change = false;
        modal.classList.add('is-active');
        for (let i = 0; i < forms.length-1; i++) {
            forms[i].value = null;
            forms[i].disabled = false;
        }
        forms[3].value = "TCP"; //SELECTがnullになると今後の処理がバグるかもしれない。
    }
}
open.addEventListener('click', modalOpen);

//「閉じるボタン」をクリックしてモーダルを閉じる
function modalClose() {
    modal.classList.remove('is-active');
}
close.addEventListener('click', modalClose);

//「モーダルの外側」をクリックしてモーダルを閉じる
function modalOut(e) {
    if (e.target === modal) {
        modal.classList.remove('is-active');
    }
}
addEventListener('click', modalOut);

//必須項目がすべて入力さてたかを確認。入力されていたら適応ボタンを有効化
const send_port_setting_element = document.getElementById("send_port_setting");
function update(){
    let already_entered = 0;
    //すべてが入力されていればalready_enteredがformsのlengthと同じになる
    for(let i = 0; i < forms.length; i++){
        if(forms[i].checkValidity()){
            already_entered++;
        }
    }

    //マウスポインタの制御
    if(already_entered === forms.length){
        send_port_setting_element.style.cursor = "pointer";
    }else {
        send_port_setting_element.style.cursor = "default";
    }
}

//フォームのイベントをリッスンする
for(let i = 0; i < forms.length; i++){
    forms[i].addEventListener("input", update);
    forms[i].addEventListener("change", update);
}

let error = 0;
//適応ボタンが押されたとき
function onApplyClick(){
    let already_entered = 0;
    //すべてが入力されていればalready_enteredがformsのlengthと同じになる
    for(let i = 0; i < forms.length; i++){
        if(forms[i].checkValidity()){
            already_entered++;
        }
    }
    if(already_entered !== forms.length){
        alert("すべての項目を入力する必要があります。");
    }else {
        xhr.open("POST", "/applyPorts");
        xhr.setRequestHeader('content-type', 'application/json;charset=UTF-8');
        xhr.send(JSON.stringify({
            "server": current_server,
            "name": forms[0].value,
            "ipaddress": forms[1].value,
            "port": Number(forms[2].value),
            "protocol": document.getElementById("protocol-field").value,
            "change": is_change
        }));
        xhr.onreadystatechange = function (){
            if(xhr.readyState === 4){
                xhr.onreadystatechange = null;
                const dialog = document.getElementById("setting-commit-status-dialog");
                dialog.querySelector("p").remove();
                const p = document.createElement("p");
                error = Number(xhr.responseText)
                switch (error){
                    case 0:
                        p.append("変更が適応されました。");
                        break;
                    case 1:
                        p.append("SQL実行時にエラーが発生しました。");
                        break;
                    case 2:
                        p.append("名前またはポート番号が重複しています。");
                        break;
                }
                dialog.prepend(p);
                dialog.showModal();
            }
        }
    }
}

send_port_setting_element.addEventListener("click", onApplyClick);

function onDialogClick(e){
    if(error===0) modalClose();
}

document.getElementById("setting-commit-status-dialog-btn").addEventListener("click", function (event){
    onDialogClick(event);
    refresh(document.getElementById(current_server));
});
