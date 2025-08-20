#include "LoginDialog.h"
#include <QMessageBox>
#include <QCryptographicHash>

LoginDialog::LoginDialog(DatabaseManager *dbManager, QWidget *parent)
    : QDialog(parent), dbManager(dbManager), userId(-1), userLevel(-1) {
    setWindowTitle("Gerenciamento de EPI - Login");
    setFixedSize(400, 300);
    setStyleSheet(
        "QWidget { background-color: #f5f5f5; font-family: 'Segoe UI'; }"
        "QLineEdit { padding: 8px; border: 2px solid #ddd; border-radius: 5px; background-color: white; font-size: 14px; }"
        "QLineEdit:focus { border-color: #4CAF50; }"
        "QPushButton { padding: 10px 20px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #45a049; }"
        "QLabel { font-size: 14px; }"
    );

    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *title = new QLabel("GERENCIADOR DE EPI");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #2196F3; margin: 20px;");
    layout->addWidget(title);

    QFormLayout *form = new QFormLayout;
    usernameEdit = new QLineEdit;
    usernameEdit->setPlaceholderText("Digite sua matrícula");
    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("Digite sua senha (4 dígitos)");
    form->addRow("Matricula:", usernameEdit);
    form->addRow("Senha:", passwordEdit);
    layout->addLayout(form);

    QPushButton *loginBtn = new QPushButton("Entrar");
    loginBtn->setToolTip("Fazer login no sistema");
    connect(loginBtn, &QPushButton::clicked, this, &LoginDialog::login);
    layout->addWidget(loginBtn);
}

void LoginDialog::login() {
    QString username = usernameEdit->text();
    QString password = QString(QCryptographicHash::hash(passwordEdit->text().toUtf8(), QCryptographicHash::Sha256).toHex());
    QVariantList result;
    if (dbManager->executeQuery(
            "SELECT id, level FROM usuarios WHERE nome_usuario = ? AND senha = ?",
            {username, password}, true, &result)) {
        if (!result.isEmpty()) {
            userId = result[0][0].toInt();
            userLevel = result[0][1].toInt();
            accept();
        } else {
            QMessageBox::warning(this, "Erro", "Credenciais inválidas!");
        }
    } else {
        QMessageBox::critical(this, "Erro", "Erro ao autenticar usuário!");
    }
}
