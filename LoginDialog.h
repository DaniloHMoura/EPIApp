#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include "DatabaseManager.h"

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(DatabaseManager *dbManager, QWidget *parent = nullptr);
    int getUserId() const { return userId; }
    int getUserLevel() const { return userLevel; }

private slots:
    void login();

private:
    DatabaseManager *dbManager;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    int userId;
    int userLevel;
};

#endif // LOGINDIALOG_H
