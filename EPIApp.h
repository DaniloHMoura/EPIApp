#ifndef EPIAPP_H
#define EPIAPP_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QPushButton>
#include <QToolBar>
#include <QChartView>
#include "DatabaseManager.h"
#include "LoginDialog.h"

class EPIApp : public QMainWindow {
    Q_OBJECT
public:
    explicit EPIApp(QWidget *parent = nullptr);

private slots:
    void addUser();
    void deleteUser();
    void addItem();
    void updateItem();
    void deleteItem();
    void addCategory();
    void updateCategory();
    void deleteCategory();
    void addEmpresa();
    void updateEmpresa();
    void deleteEmpresa();
    void filterItems();
    void loadItemlkDetails();
    void onColaboradorSelected();
    void onMovNameChanged(const QString &text);
    void onMovSizeSelected();
    void addToPending();
    void removePending(int row);
    void confirmPending();
    void onReturnColabSelected();
    void addReturnsToPending();
    void removeReturnPending(int row);
    void confirmReturns();
    void loadDelivered();
    void generateLowStockReport();
    void generateInventoryReport();
    void generateCategoryReport();
    void showMostUsedGraph();
    void exportToCsv();
    void exportToPdf();
    void logout();
    void refreshAllData();
    void loadCategoryDetails(QListWidgetItem *item);
    void loadEmpresaDetails(QListWidgetItem *item);

private:
    void setupUi();
    QWidget* createManagementTab();
    QWidget* createDashboardTab();
    QWidget* createItemsTab();
    QWidget* createWithdrawalTab();
    QWidget* createReturnTab();
    QWidget* createDeliveredTab();
    QWidget* createCategoriesTab();
    QWidget* createEmpresasTab();
    QWidget* createReportsTab();
    QWidget* createAboutTab();
    QWidget* createRestrictedTab(const QString &message);
    void createToolbar();
    bool checkAdmin(const QString &action);
    void loadItems();
    void loadCategories();
    void loadUsers();
    void loadColaboradores();
    void loadDeliveredColab();
    void loadColabFilter();
    void loadEmpresas();
    void loadEmpresasList();
    void updateCompleters();
    void updatePendingTable();
    void updateReturnPendingTable();
    void logAudit(const QString &action, const QString &details);
    void handleError(const QString &action, const QString &error, const QString &message = "Ocorreu um erro inesperado");
    bool confirmAction(const QString &message);
    QString getDateRange();
    void clearUserForm();
    void clearItemForm();
    void clearCategoryForm();
    void clearEmpresaForm();

    DatabaseManager *dbManager;
    int currentUserId;
    int currentUserLevel;
    QList<QVariantList> pendingWithdrawals;
    QList<QVariantList> pendingReturns;

    // UI Components
    QTableWidget *usersTable;
    QLineEdit *userNomeCompleto;
    QLineEdit *userMatricula;
    QLineEdit *userCpf;
    QLineEdit *userSenha;
    QComboBox *userLevel;
    QComboBox *userEmpresa;
    QTableWidget *itemsTable;
    QLineEdit *searchInput;
    QComboBox *categoryFilter;
    QLineEdit *itemName;
    QLineEdit *itemCa;
    QLineEdit *itemSize;
    QLineEdit *itemBrand;
    QComboBox *itemCategory;
    QSpinBox *itemQuantity;
    QDoubleSpinBox *itemPrice;
    QSpinBox *itemMinStock;
    QLineEdit *itemSupplier;
    QComboBox *colaboradorCombo;
    QLineEdit *movName;
    QComboBox *movSize;
    QLineEdit *movCa;
    QComboBox *movCategory;
    QSpinBox *movWithdrawQty;
    QSpinBox *movValidDays;
    QTableWidget *pendingTable;
    QComboBox *returnColabCombo;
    QTableWidget *withdrawnTable;
    QTableWidget *returnPendingTable;
    QComboBox *deliveredColab;
    QLineEdit *deliveredStartDel;
    QLineEdit *deliveredEndDel;
    QLineEdit *deliveredStartExp;
    QLineEdit *deliveredEndExp;
    QTableWidget *deliveredTable;
    QListWidget *categoriesList;
    QLineEdit *categoryName;
    QTextEdit *categoryDescription;
    QListWidget *empresasList;
    QLineEdit *empresaNome;
    QLineEdit *empresaCnpj;
    QLineEdit *empresaLogadouro;
    QComboBox *timeFilter;
    QComboBox *collabFilter;
    QLineEdit *startDate;
    QLineEdit *endDate;
    QTextEdit *reportDisplay;
    QtCharts::QChartView *chartView;
};

#endif // EPIAPP_H
