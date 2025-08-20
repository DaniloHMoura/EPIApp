#include "EPIApp.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDateTime>
#include <QCompleter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QHeaderView>
#include <QChart>
#include <QBarSeries>
#include <QBarSet>
#include <QValueAxis>
#include <QPdfWriter>
#include <QPainter>
#include <QDir>

EPIApp::EPIApp(QWidget *parent) : QMainWindow(parent), dbManager(new DatabaseManager), currentUserId(-1), currentUserLevel(-1) {
    setStyleSheet(
        "QWidget { background-color: #f5f5f5; font-family: 'Segoe UI'; }"
        "QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox { padding: 8px; border: 2px solid #ddd; border-radius: 5px; background-color: white; font-size: 14px; }"
        "QLineEdit:focus, QComboBox:focus { border-color: #4CAF50; }"
        "QPushButton { padding: 10px 20px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #45a049; }" 
        "QPushButton:pressed { background-color: #3d8b40; }"
        "QPushButton[delete=true] { background-color: #f44336; }"
        "QTableWidget { gridline-color: #ddd; background-color: white; alternate-background-color: #f9f9f9; }"
        "QTableWidget::item:selected { background-color: #4CAF50; color: white; }"
        "QHeaderView::section { background-color: #2196F3; color: white; padding: 10px; font-weight: bold; border: none; }"
        "QTabWidget::pane { border: 1px solid #ddd; background-color: white; }"
        "QTabBar::tab { background-color: #e0e0e0; padding: 10px 20px; margin-right: 2px; }"
        "QTabBar::tab:selected { background-color: #4CAF50; color: white; }"
        "QGroupBox { font-weight: bold; border: 2px solid #ddd; border-radius: 5px; margin: 10px; padding-top: 10px; }"
        "QLabel { font-size: 14px; }"
    );

    LoginDialog loginDialog(dbManager);
    if (loginDialog.exec() != QDialog::Accepted) {
        exit(0);
    }
    currentUserId = loginDialog.getUserId();
    currentUserLevel = loginDialog.getUserLevel();

    setupUi();
}

void EPIApp::setupUi() {
    setWindowTitle("Sistema Avançado de Gerenciamento de EPI");
    resize(1200, 800);

    QTabWidget *centralWidget = new QTabWidget(this);
    setCentralWidget CENTRALWidget);
    centralWidget->addTab(createManagementTab(), "Colaboradores");
    centralWidget->addTab(createDashboardTab(), "Painel");
    centralWidget->addTab(createItemsTab(), "Itens");
    centralWidget->addTab(createWithdrawalTab(), "Retirada");
    centralWidget->addTab(createReturnTab(), "Devolução");
    centralWidget->addTab(createDeliveredTab(), "EPIs Entregues");
    centralWidget->addTab(createCategoriesTab(), "Categorias");
    centralWidget->addTab(createEmpresasTab(), "Empresas");
    centralWidget->addTab(createReportsTab(), "Relatórios");
    centralWidget->addTab(createAboutTab(), "Sobre");

    createToolbar();
    statusBar()->showMessage("Pronto! Sistema desenvolvido por Danilo Hollanders de Moura.");
    refreshAllData();
}

QWidget* EPIApp::createManagementTab() {
    if (!checkAdmin("gerenciar usuários")) {
        return createRestrictedTab("Apenas administradores podem gerenciar usuários.");
    }

    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(widget);

    usersTable = new QTableWidget(0, 6);
    usersTable->setHorizontalHeaderLabels({"ID", "Nome Completo", "Matrícula", "CPF", "Nível", "Empresa"});
    usersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    usersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    usersTable->setAlternatingRowColors(true);

    QGroupBox *formGroup = new QGroupBox("Cadastrar Colaborador");
    QFormLayout *formLayout = new QFormLayout;
    userNomeCompleto = new QLineEdit;
    userNomeCompleto->setPlaceholderText("Digite o nome completo");
    userMatricula = new QLineEdit;
    userMatricula->setPlaceholderText("Digite a matrícula");
    userCpf = new QLineEdit;
    userCpf->setPlaceholderText("Digite o CPF");
    userSenha = new QLineEdit;
    userSenha->setPlaceholderText("Digite a senha (4 dígitos)");
    userSenha->setEchoMode(QLineEdit::Password);
    userSenha->setInputMask("0000");
    userLevel = new QComboBox;
    userLevel->addItems({"Colaborador (1)", "Almoxarife (2)", "Admin (3)"});
    userEmpresa = new QComboBox;
    userEmpresa->addItem("Selecionar Empresa", 0);

    formLayout->addRow("Nome Completo:", userNomeCompleto);
    formLayout->addRow("Matrícula:", userMatricula);
    formLayout->addRow("CPF:", userCpf);
    formLayout->addRow("Senha (4 dígitos):", userSenha);
    formLayout->addRow("Nível:", userLevel);
    formLayout->addRow("Empresa:", userEmpresa);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    QPushButton *addBtn = new QPushButton("Adicionar Colaborador");
    addBtn->setToolTip("Adicionar um novo Colaborador");
    connect(addBtn, &QPushButton::clicked, this, &EPIApp::addUser);
    QPushButton *deleteBtn = new QPushButton("Deletar Colaborador");
    deleteBtn->setToolTip("Deletar o Colaborador selecionado");
    deleteBtn->setProperty("delete", true);
    connect(deleteBtn, &QPushButton::clicked, this, &EPIApp::deleteUser);
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(deleteBtn);
    formLayout->addRow(btnLayout);
    formGroup->setLayout(formLayout);

    layout->addWidget(usersTable);
    layout->addWidget(formGroup);
    return widget;
}

QWidget* EPIApp::createDashboardTab() {
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(widget);

    chartView = new QtCharts::QChartView;
    QVariantList items;
    dbManager->executeQuery(
        "SELECT nome, quantidade FROM itens WHERE quantidade > 0 ORDER BY quantidade DESC LIMIT 10",
        {}, true, &items);
    QtCharts::QBarSeries *series = new QtCharts::QBarSeries;
    Qt DitCharts::QBarSet *set = new QtCharts::QBarSet("Quantidade");
    for (const auto &item : items) {
        set->append(item[1].toInt());
    }
    QtCharts::QChart *chart = new QtCharts::QChart;
    chart->addSeries(series);
    chart->setTitle("Níveis de Estoque de EPIs");
    QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis;
    axisY->setTitleText("Quantidade");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    QStringList categories;
    for (const auto &item : items) {
        categories.append(item[0].toString());
    }
    QtCharts::QBarCategoryAxis *axisX = new QtCharts::QBarCategoryAxis;
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    chartView->setChart(chart);
    layout->addWidget(chartView);
    return widget;
}

QWidget* EPIApp::createItemsTab() {
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchInput = new QLineEdit;
    searchInput->setPlaceholderText("Pesquisar EPIs por nome ou CA...");
    connect(searchInput, &QLineEdit::textChanged, this, &EPIApp::filterItems);
    categoryFilter = new QComboBox;
    categoryFilter->addItem("Todas as Categorias");
    connect(categoryFilter, &QComboBox::currentTextChanged, this, &EPIApp::filterItems);
    searchLayout->addWidget(new QLabel("Pesquisar:"));
    searchLayout->addWidget(searchInput);
    searchLayout->addWidget(new QLabel("Categoria:"));
    searchLayout->addWidget(categoryFilter);

    itemsTable = new QTableWidget(0, 11);
    itemsTable->setHorizontalHeaderLabels({"ID", "Nome", "CA", "Tamanho", "Marca", "Categoria", "Quantidade", "Preço", "Estoque Mínimo", "Fornecedor", "Data de Adição"});
    itemsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    itemsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    itemsTable->setAlternatingRowColors(true);
    connect(itemsTable, &QTableWidget::itemSelectionChanged, this, &EPIApp::loadItemDetails);

    QGroupBox *formGroup = new QGroupBox("Adicionar/Editar EPI");
    QFormLayout *formLayout = new QFormLayout;
    itemName = new QLineEdit;
    itemName->setPlaceholderText("Digite o nome do EPI...");
    itemCa = new QLineEdit;
    itemSize = new QLineEdit;
    itemBrand = new QLineEdit;
    itemCategory = new QComboBox;
    itemQuantity = new QSpinBox;
    itemQuantity->setMaximum(999999);
    itemPrice = new QDoubleSpinBox;
    itemPrice->setMaximum(999999.99);
    itemPrice->setDecimals(2);
    itemMinStock = new QSpinBox;
    itemMinStock->setMaximum(999999);
    itemSupplier = new QLineEdit;

    formLayout->addRow("Nome:", itemName);
    formLayout->addRow("CA:", itemCa);
    formLayout->addRow("Tamanho:", itemSize);
    formLayout->addRow("Marca:", itemBrand);
    formLayout->addRow("Categoria:", itemCategory);
    formLayout->addRow("Quantidade:", itemQuantity);
    formLayout->addRow("Preço:", itemPrice);
    formLayout->addRow("Estoque Mínimo:", itemMinStock);
    formLayout->addRow("Fornecedor:", itemSupplier);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    QPushButton *addBtn = new QPushButton("Adicionar EPI");
    addBtn->setToolTip("Adicionar um novo EPI ao estoque");
    connect(addBtn, &QPushButton::clicked, this, &EPIApp::addItem);
    QPushButton *updateBtn = new QPushButton("Atualizar EPI");
    updateBtn->setToolTip("Atualizar o EPI selecionado");
    connect(updateBtn, &QPushButton::clicked, this, &EPIApp::updateItem);
    QPushButton *deleteBtn = new QPushButton("Deletar EPI");
    deleteBtn->setToolTip("Deletar o EPI selecionado");
    deleteBtn->setProperty("delete", true);
    connect(deleteBtn, &QPushButton::clicked, this, &EPIApp::deleteItem);
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(updateBtn);
    btnLayout->addWidget(deleteBtn);
    formLayout->addRow(btnLayout);
    formGroup->setLayout(formLayout);

    layout->addLayout(searchLayout);
    layout->addWidget(itemsTable);
    layout->addWidget(formGroup);
    return widget;
}

QWidget* EPIApp::createWithdrawalTab() {
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QGroupBox *movGroup = new QGroupBox("Retirada de EPI");
    QFormLayout *movLayout = new QFormLayout;
    colaboradorCombo = new QComboBox;
    connect(colaboradorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EPIApp::onColaboradorSelected);
    movName = new QLineEdit;
    movName->setPlaceholderText("Digite o nome do EPI ou CA...");
    connect(movName, &QLineEdit::textChanged, this, &EPIApp::onMovNameChanged);
    movSize = new QComboBox;
    connect(movSize, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EPIApp::onMovSizeSelected);
    movCa = new QLineEdit;
    movCa->setReadOnly(true);
    movCategory = new QComboBox;
    movCategory->setEnabled(false);
    movWithdrawQty = new QSpinBox;
    movWithdrawQty->setMinimum(1);
    movWithdrawQty->setMaximum(999999);
    movValidDays = new QSpinBox;
    movValidDays->setMinimum(0);
    movValidDays->setMaximum(3650);
    movValidDays->setValue(180);

    movLayout->addRow("Colaborador:", colaboradorCombo);
    movLayout->addRow("Pesquisar EPI/CA:", movName);
    movLayout->addRow("Tamanho:", movSize);
    movLayout->addRow("CA:", movCa);
    movLayout->addRow("Categoria:", movCategory);
    movLayout->addRow("Quantidade a Retirar:", movWithdrawQty);
    movLayout->addRow("Validade (dias):", movValidDays);

    QPushButton *addToListBtn = new QPushButton("Adicionar à Lista de Retirada");
    addToListBtn->setToolTip("Adicionar o EPI selecionado à lista de retirada");
    connect(addToListBtn, &QPushButton::clicked, this, &EPIApp::addToPending);
    movLayout->addRow(addToListBtn);

    pendingTable = new QTableWidget(0, 5);
    pendingTable->setHorizontalHeaderLabels({"Nome", "CA", "Tamanho", "Quantidade", "Ação"});
    pendingTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QPushButton *confirmAllBtn = new QPushButton("Confirmar Todas as Retiradas");
    confirmAllBtn->setToolTip("Confirmar todas as retiradas pendentes");
    connect(confirmAllBtn, &QPushButton::clicked, this, &EPIApp::confirmPending);
    movLayout->addRow(pendingTable);
    movLayout->addRow(confirmAllBtn);
    movGroup->setLayout(movLayout);
    layout->addWidget(movGroup);
    return widget;
}

QWidget* EPIApp::createReturnTab() {
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QGroupBox *retGroup = new QGroupBox("Devolução de EPI");
    QFormLayout *retLayout = new QFormLayout;
    returnColabCombo = new QComboBox;
    connect(returnColabCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EPIApp::onReturnColabSelected);
    retLayout->addRow("Colaborador:", returnColabCombo);

    withdrawnTable = new QTableWidget(0, 6);
    withdrawnTable->setHorizontalHeaderLabels({"ID", "Nome", "CA", "Tamanho", "Qnt Pendente", "Qnt a Devolver"});
    withdrawnTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    retLayout->addRow(withdrawnTable);

    QPushButton *addToPendingBtn = new QPushButton("Adicionar à Lista de Devolução");
    connect(addToPendingBtn, &QPushButton::clicked, this, &EPIApp::addReturnsToPending);
    retLayout->addRow(addToPendingBtn);

    returnPendingTable = new QTableWidget(0, 5);
    returnPendingTable->setHorizontalHeaderLabels({"Nome", "CA", "Tamanho", "Quantidade", "Ação"});
    returnPendingTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    retLayout->addRow(returnPendingTable);

    QPushButton *confirmReturnsBtn = new QPushButton("Confirmar Devoluções");
    connect(confirmReturnsBtn, &QPushButton::clicked, this, &EPIApp::confirmReturns);
    retLayout->addRow(confirmReturnsBtn);

    retGroup->setLayout(retLayout);
    layout->addWidget(retGroup);
    return widget;
}

QWidget* EPIApp::createDeliveredTab() {
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QGroupBox *filterGroup = new QGroupBox("Filtros");
    QFormLayout *filterLayout = new QFormLayout;
    deliveredColab = new QComboBox;
    deliveredColab->addItem("Todos", 0);
    deliveredStartDel = new QLineEdit;
    deliveredStartDel->setPlaceholderText("Início Entrega (AAAA-MM-DD)");
    deliveredEndDel = new QLineEdit;
    deliveredEndDel->setPlaceholderText("Fim Entrega (AAAA-MM-DD)");
    deliveredStartExp = new QLineEdit;
    deliveredStartExp->setPlaceholderText("Início Vencimento (AAAA-MM-DD)");
    deliveredEndExp = new QLineEdit;
    deliveredEndExp->setPlaceholderText("Fim Vencimento (AAAA-MM-DD)");
    filterLayout->addRow("Colaborador:", deliveredColab);
    filterLayout->addRow("Início Entrega:", deliveredStartDel);
    filterLayout->addRow("Fim Entrega:", deliveredEndDel);
    filterLayout->addRow("Início Vencimento:", deliveredStartExp);
    filterLayout->addRow("Fim Vencimento:", deliveredEndExp);
 śl

    QPushButton *filterBtn = new QPushButton("Filtrar");
    connect(filterBtn, &QPushButton::clicked, this, &EPIApp::loadDelivered);
    filterLayout->addRow(filterBtn);
    filterGroup->setLayout(filterLayout);
    layout->addWidget(filterGroup);

    deliveredTable = new QTableWidget(0, 7);
    deliveredTable->setHorizontalHeaderLabels({"Colaborador", "Nome EPI", "CA", "Tamanho", "Quantidade", "Data Entrega", "Data Vencimento"});
    deliveredTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(deliveredTable);
    return widget;
}

QWidget* EPIApp::createCategoriesTab() {
    QWidget *widget = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(widget);

    categoriesList = new QListWidget;
    connect(categoriesList, &QListWidget::itemClicked, this, &EPIApp::loadCategoryDetails);

    QGroupBox *formGroup = new QGroupBox("Adicionar/Editar Categoria");
    QFormLayout *formLayout = new QFormLayout;
    categoryName = new QLineEdit;
    categoryDescription = new QTextEdit;
    categoryDescription->setMaximumHeight(100);
    formLayout->addRow("Nome:", categoryName);
    formLayout->addRow("Descrição:", categoryDescription);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    QPushButton *addBtn = new QPushButton("Adicionar Categoria");
    addBtn->setToolTip("Adicionar uma nova categoria");
    connect(addBtn, &QPushButton::clicked, this, &EPIApp::addCategory);
    QPushButton *updateBtn = new QPushButton("Atualizar Categoria");
    updateBtn->setToolTip("Atualizar a categoria selecionada");
    connect(updateBtn, &QPushButton::clicked, this, &EPIApp::updateCategory);
    QPushButton *deleteBtn = new QPushButton("Deletar Categoria");
    deleteBtn->setToolTip("Deletar a categoria selecionada");
    deleteBtn->setProperty("delete", true);
    connect(deleteBtn, &QPushButton::clicked, this, &EPIApp::deleteCategory);
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(updateBtn);
    btnLayout->addWidget(deleteBtn);
    formLayout->addRow(btnLayout);
    formGroup->setLayout(formLayout);

    layout->addWidget(categoriesList);
    layout->addWidget(formGroup);
    return widget;
}

QWidget* EPIApp::createEmpresasTab() {
    if (!checkAdmin("gerenciar empresas")) {
        return createRestrictedTab("Apenas administradores podem gerenciar empresas.");
    }

    QWidget *widget = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(widget);

    empresasList = new QListWidget;
    connect(empresasList, &QListWidget::itemClicked, this, &EPIApp::loadEmpresaDetails);

    QGroupBox *formGroup = new QGroupBox("Cadastrar/Editar Empresa");
    QFormLayout *formLayout = new QFormLayout;
    empresaNome = new QLineEdit;
    empresaNome->setPlaceholderText("Digite o nome da empresa");
    empresaCnpj = new QLineEdit;
    empresaCnpj->setPlaceholderText("Digite即 o CNPJ");
    empresaLogadouro = new QLineEdit;
    empresaLogadouro->setPlaceholderText("Digite o logadouro");
    formLayout->addRow("Nome:", empresaNome);
    formLayout->addRow("CNPJ:", empresaCnpj);
    formLayout->addRow("Logadouro:", empresaLogadouro);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    QPushButton *addBtn = new QPushButton("Adicionar Empresa");
    addBtn->setToolTip("Adicionar uma nova empresa");
    connect(addBtn, &QPushButton::clicked, this, &EPIApp::addEmpresa);
    QPushButton *updateBtn = new QPushButton("Atualizar Empresa");
    updateBtn->setToolTip("Atualizar a empresa selecionada");
    connect(updateBtn, &QPushButton::clicked, this, &EPIApp::updateEmpresa);
    QPushButton *deleteBtn = new QPushButton("Deletar Empresa");
    deleteBtn->setToolTip("Deletar a empresa selecionada");
    deleteBtn->setProperty("delete", true);
    connect(deleteBtn, &QPushButton::clicked, this, &EPIApp::deleteEmpresa);
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(updateBtn);
    btnLayout->addWidget(deleteBtn);
    formLayout->addRow(btnLayout);
    formGroup->setLayout(formLayout);

    layout->addWidget(empresasList);
    layout->addWidget(formGroup);
    return widget;
}

QWidget* EPIApp::createReportsTab() {
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QHBoxLayout *filterLayout = new QHBoxLayout;
    time WarnFilter = new QComboBox;
    timeFilter->addItems({"1 Dia", "1 Semana", "15 Dias", "1 Mês", "3 Meses", "6 Meses", "1 Ano", "5 Anos", "10 Anos", "Todos os Períodos"});
    timeFilter->setCurrentText("Todos os Períodos");
    collabFilter = new QComboBox;
    collabFilter->addItem("Selecionar Colaborador", 0);
    startDate = new QLineEdit;
    startDate->setPlaceholderText("Início (AAAA-MM-DD)");
    endDate = new QLineEdit;
    endDate->setPlaceholderText("Fim (AAAA-MM-DD)");
    filterLayout->addWidget(new QLabel("Período:"));
    filterLayout->addWidget(timeFilter);
    filterLayout->addWidget(new QLabel("Colaborador:"));
    filterLayout->addWidget(collabFilter);
    filterLayout->addWidget(new QLabel("Início:"));
    filterLayout->addWidget(startDate);
    filterLayout->addWidget(new QLabel("Fim:"));
    filterLayout->addWidget(endDate);
    filterLayout->addStretch();

    QHBoxLayout *reportLayout = new QHBoxLayout;
    struct Button {
        QString text;
        void (EPIApp::*slot)();
        QString tooltip;
    };
    Button buttons[] = {
        {"Relatório de Estoque Baixo", &EPIApp::generateLowStockReport, "Gerar relatório de EPIs com estoque baixo"},
        {"Relatório Completo de EPIs", &EPIApp::generateInventoryReport, "Gerar relatório completo do estoque de EPIs"},
        {"Relatório por Categoria", &EPIApp::generateCategoryReport, "Gerar relatório por categoria de EPIs"},
        {"Gráfico de EPIs Mais Usados", &EPIApp::showMostUsedGraph, "Exibir gráfico dos EPIs mais retirados"}
    };
    for (const auto &btn : buttons) {
        QPushButton *button = new QPushButton(btn.text);
        button->setToolTip(btn.tooltip);
        connect(button, &QPushButton::clicked, this, btn.slot);
        reportLayout->addWidget(button);
    }

    reportDisplay = new QTextEdit;
    reportDisplay->setReadOnly(true);
    layout->addLayout(filterLayout);
    layout->addLayout(reportLayout);
    layout->addWidget(reportDisplay);
    return widget;
}

QWidget* EPIApp::createAboutTab() {
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(widget);
    QGroupBox *aboutGroup = new QGroupBox("Sobre o Sistema");
    QVBoxLayout *aboutLayout = new QVBoxLayout;

    QStringList info = {
        "Sistema Avançado de Gerenciamento de EPI|font-size: 20px; font-weight: bold; color: #2196F3; margin: 20px;",
        "Versão: 1.2|",
        "Desenvolvedor: Danilo Hollanders de Moura|",
        "Email: danilo.aax@gmail.com|",
        "Telefone: (34) 99209-1807|"
    };
    for (const auto &line : info) {
        QStringList parts = line.split("|");
        QLabel *label = new QLabel(parts[0]);
        label->setAlignment(Qt::AlignCenter);
        if (parts.size() > 1 && !parts[1].isEmpty()) {
            label->setStyleSheet(parts[1]);
        }
        aboutLayout->addWidget(label);
    }
    aboutLayout->addStretch();
    aboutGroup->setLayout(aboutLayout);
    layout->addWidget(aboutGroup);
    layout->addStretch();
    return widget;
}

QWidget* EPIApp::createRestrictedTab(const QString &message) {
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->addWidget(new QLabel(message));
    return widget;
}

void EPIApp::createToolbar() {
    QToolBar *toolbar = addToolBar("Principal");
    struct Action {
        QString text;
        QString tooltip;
        void (EPIApp::*slot)();
    };
    Action actions[] = {
        {"Atualizar", "Atualizar todos os dados", &EPIApp::refreshAllData},
        {"Exportar CSV", "Exportar dados de EPIs para CSV", &EPIApp::exportToCsv},
        {"Exportar PDF", "Exportar dados de EPIs para PDF", &EPIApp::exportToPdf},
        {"Sair", "Sair do sistema", &EPIApp::logout}
    };
    for (int i = 0; i < 4; ++i) {
        QAction *action = new QAction(actions[i].text, this);
        action->setToolTip(actions[i].tooltip);
        connect(action, &QAction::triggered, this, actions[i].slot);
        toolbar->addAction(action);
        if (i < 3) {
            toolbar->addSeparator();
        }
    }
}

bool EPIApp::checkAdmin(const QString &action) {
    if (currentUserLevel != 3) {
        QMessageBox::warning(this, "Erro", QString("Apenas administradores podem %1!").arg(action));
        return false;
    }
    return true;
}

void EPIApp::addUser() {
    if (!checkAdmin("adicionar usuários")) return;

    QString nomeCompleto = userNomeCompleto->text();
    QString matricula = userMatricula->text();
    QString cpf = userCpf->text();
    QString senha = userSenha->text();
    int level = userLevel->currentText().split("(")[1][0].digitValue();
    int empresaId = userEmpresa->currentData().toInt();

    if (nomeCompleto.isEmpty() || matricula.isEmpty() || cpf.isEmpty() || senha.length() != 4 || empresaId == 0) {
        QMessageBox::warning(this, "Erro", "Preencha todos os campos! Senha deve ter 4 dígitos e Empresa deve ser selecionada.");
        return;
    }

    if (!confirmAction(QString("Adicionar usuário '%1' (Matrícula: %2)?").arg(nomeCompleto, matricula))) {
        return;
    }

    QString hashedSenha = QString(QCryptographicHash::hash(senha.toUtf8(), QCryptographicHash::Sha256).toHex());
    if (dbManager->executeQuery(
            "INSERT INTO usuarios (nome_usuario, senha, level, nome_completo, matricula, cpf, empresa_id) "
            "VALUES (?, ?, ?, ?, ?, ?, ?)",
            {matricula, hashedSenha, level, nomeCompleto, matricula, cpf, empresaId})) {
        clearUserForm();
        loadUsers();
        loadColaboradores();
        logAudit("add_user", QString("Adicionou usuário '%1' (Matrícula: %2, Empresa ID: %3)").arg(nomeCompleto, matricula, QString::number(empresaId)));
        QMessageBox::information(this, "Sucesso", QString("Usuário '%1' adicionado com sucesso!").arg(nomeCompleto));
    } else {
        QMessageBox::critical(this, "Erro", "Falha ao adicionar usuário. Verifique matrícula/CPF.");
    }
}

void EPIApp::deleteUser() {
    if (!checkAdmin("deletar usuários")) return;

    int currentRow = usersTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erro", "Selecione um usuário para deletar!");
        return;
    }

    QString userId = usersTable->item(currentRow, 0)->text();
    QString nomeCompleto = usersTable->item(currentRow, 1)->text();
    QString matricula = usersTable->item(currentRow, 2)->text();

    if (userId.toInt() == currentUserId) {
        QMessageBox::warning(this, "Erro", "Você não pode deletar seu próprio usuário!");
        return;
    }

    if (!confirmAction(QString("Deletar usuário '%1' (Matrícula: %2)?").arg(nomeCompleto, matricula))) {
        return;
    }

    if (dbManager->executeQuery("DELETE FROM usuarios WHERE id=?", {userId})) {
        loadUsers();
        loadColaboradores();
        logAudit("delete_user", QString("Deletou usuário '%1' (Matrícula: %2)").arg(nomeCompleto, matricula));
        QMessageBox::information(this, "Sucesso", QString("Usuário '%1' deletado com sucesso!").arg(nomeCompleto));
    } else {
        QMessageBox::critical(this, "Erro", "Falha ao deletar usuário.");
    }
}

void EPIApp::addItem() {
    if (currentUserLevel != 2 && currentUserLevel != 3) {
        QMessageBox::warning(this, "Erro", "Apenas almoxarifes ou administradores podem adicionar EPIs!");
        return;
    }

    QString itemNameText = itemName->text();
    int categoryId = itemCategory->currentData().toInt();
    if (itemNameText.isEmpty() || categoryId == 0) {
        QMessageBox::warning(this, "Erro", "Nome do EPI e categoria são obrigatórios!");
        return;
    }

    if (!confirmAction(QString("Adicionar EPI '%1'?").arg(itemNameText))) {
        return;
    }

    if (dbManager->executeQuery(
            "INSERT INTO itens (nome, ca, tamanho, marca, categoria_id, quantidade, preco, estoque_minimo, fornecedor, data_adicao) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
            {itemNameText, itemCa->text(), itemSize->text(), itemBrand->text(), categoryId,
             itemQuantity->value(), itemPrice->value(), itemMinStock->value(), itemSupplier->text(),
             QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")})) {
        clearItemForm();
        loadItems();
        updateCompleters();
        logAudit("add_item", QString("Adicionou EPI '%1'").arg(itemNameText));
        QMessageBox::information(this, "Sucesso", QString("EPI '%1' adicionado com sucesso!").arg(itemNameText));
    } else {
        QMessageBox::critical(this, "Erro", "Falha ao adicionar EPI. Nome ou CA já existe.");
    }
}

void EPIApp::updateItem() {
    if (currentUserLevel != 2 && currentUserLevel != 3) {
        QMessageBox::warning(this, "Erro", "Apenas almoxarifes ou administradores podem atualizar EPIs!");
        return;
    }

    int currentRow = itemsTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erro", "Selecione um EPI para atualizar!");
        return;
    }

    QString itemId = itemsTable->item(currentRow, 0)->text();
    QString itemNameText = itemName->text();
    int categoryId = itemCategory->currentData().toInt();
    if (itemNameText.isEmpty() || categoryId == 0) {
        QMessageBox::warning(this, "Erro", "Nome do EPI e categoria são obrigatórios!");
        return;
    }

    if (!confirmAction(QString("Atualizar EPI '%1' (ID: %2)?").arg(itemNameText, itemId))) {
        return;
    }

    if (dbManager->executeQuery(
            "UPDATE itens SET nome=?, ca=?, tamanho=?, marca=?, categoria_id=?, quantidade=?, preco=?, estoque_minimo=?, fornecedor=? WHERE id=?",
            {itemNameText, itemCa->text(), itemSize->text(), itemBrand->text(), categoryId,
             itemQuantity->value(), itemPrice->value(), itemMinStock->value(), itemSupplier->text(), itemId})) {
        clearItemForm();
        loadItems();
        updateCompleters();
        logAudit("update_item", QString("Atualizou EPI '%1' (ID: %2)").arg(itemNameText, itemId));
        QMessageBox::information(this, "Sucesso", QString("EPI '%1' atualizado com sucesso!").arg(itemNameText));
    } else {
        QMessageBox::critical(this, "Erro", "Falha ao atualizar EPI. Nome ou CA já existe.");
    }
}

void EPIApp::deleteItem() {
    if (currentUserLevel != 2 && currentUserLevel != 3) {
        QMessageBox::warning(this, "Erro", "Apenas almoxarifes ou administradores podem deletar EPIs!");
        return;
    }

    int currentRow = itemsTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erro", "Selecione um EPI para deletar!");
        return;
    }

    QString itemId = itemsTable->item(currentRow, 0)->text();
    QString itemNameText = itemsTable->item(currentRow, 1)->text();
    if (!confirmAction(QString("Deletar EPI '%1' (ID: %2)?").arg(itemNameText, itemId))) {
        return;
    }

    if (dbManager->executeQuery("DELETE FROM itens WHERE id=?", {itemId})) {
        clearItemForm();
        loadItems();
        updateCompleters();
        logAudit("delete_item", QString("Deletou EPI '%1' (ID: %2)").arg(itemNameText, itemId));
        QMessageBox::information(this, "Sucesso", QString("EPI '%1' deletado com sucesso!").arg(itemNameText));
    } else {
        QMessageBox::critical(this, "Erro", "Falha ao deletar EPI.");
    }
}

void EPIApp::addCategory() {
    if (!checkAdmin("adicionar categorias")) return;

    QString categoryNameText = categoryName->text();
    if (categoryNameText.isEmpty()) {
        QMessageBox::warning(this, "Erro", "O nome da categoria é obrigatório!");
        return;
    }

    if (!confirmAction(QString("Adicionar categoria '%1'?").arg(categoryNameText))) {
        return;
    }

    if (dbManager->executeQuery(
            "INSERT INTO categorias (nome, descricao) VALUES (?, ?)",
            {categoryNameText, categoryDescription->toPlainText()})) {
        clearCategoryForm();
        loadCategories();
        loadItems();
        logAudit("add_category", QString("Adicionou categoria '%1'").arg(categoryNameText));
        QMessageBox::information(this, "Sucesso", QString("Categoria '%1' adicionada com sucesso!").arg(categoryNameText));
    } else {
        QMessageBox::critical(this, "Erro", "Falha ao adicionar categoria. Nome já existe.");
    }
}

void EPIApp::updateCategory() {
    if (!checkAdmin("atualizar categorias")) return;

    QListWidgetItem *currentItem = categoriesList->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "Erro", "Selecione uma categoria para atualizar!");
        return;
    }

    int catId = currentItem->data(Qt::UserRole).toInt();
    QString categoryNameText = categoryName->text();
    if (categoryNameText.isEmpty()) {
        QMessageBox::warning(this, "Erro", "O nome da categoria é obrigatório!");
        return;
    }

    if (!confirmAction(QString("Atualizar categoria '%1' (ID: %2)?").arg(categoryNameText, QString::number(catId)))) {
        return;
    }

    if (dbManager->executeQuery(
            "UPDATE categorias SET nome=?, descricao=? WHERE id=?",
            {categoryNameText, categoryDescription->toPlainText(), catId})) {
        clearCategoryForm();
        loadCategories();
        loadItems();
        logAudit("update_category", QString("Atualizou categoria '%1' (ID: %2)").arg(categoryNameText, QString::number(catId)));
        QMessageBox::information(this, "Sucesso", QString("Categoria '%1' atualizada com sucesso!").arg(categoryNameText));
    } else {
        QMessageBox::critical(this, "Erro", "Falha ao atualizar categoria. Nome já existe.");
    }
}

void EPIApp::deleteCategory() {
    if (!checkAdmin("deletar categorias")) return;

    QListWidgetItem *currentItem = categoriesList->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "Erro", "Selecione uma categoria para deletar!");
        return;
    }

    int catId = currentItem->data(Qt::UserRole).toInt();
    QString categoryNameText = currentItem->text();
    if (!confirmAction(QString("Deletar categoria '%1' (ID: %2)?").arg(categoryNameText, QString::number(catId)))) {
        return;
    }

    if (dbManager->executeQuery("DELETE FROM categorias WHERE id=?", {catId})) {
        clearCategoryForm();
        loadCategories();
        loadItems();
        logAudit("delete_category", QString("Deletou categoria '%1' (ID: %2)").arg(categoryNameText, QString::number(catId)));
        QMessageBox::information(this, "Sucesso", QString("Categoria '%1' deletada com sucesso!").arg(categoryNameText));
    } else {
        QMessageBox::critical(this, "Erro", "Falha ao deletar categoria. Categoria vinculada a itens.");
    }
}

void EPIApp::addEmpresa() {
    if (!checkAdmin("adicionar empresas")) return;

    QString nome = empresaNome->text();
    QString cnpj = empresaCnpj->text();
    QString logadouro = empresaLogkostro->text();
    if (nome.isEmpty() || cnpj.isEmpty()) {
        QMessageBox::warning(this, "Erro", "Nome e CNPJ são obrigatórios!");
        return;
    }

    if (!confirmAction(QString("Adicionar empresa '%1' (CNPJ: %2)?").arg(nome, cnpj))) {
        return;
    }

    if (dbManager->executeQuery(
            "INSERT INTO empresas (nome, cnpj, logadouro) VALUES (?, ?, ?)",
            {nome, cnpj, logadouro})) {
        clearEmpresaForm();
        loadEmpresasList();
        loadEmpresas();
        logAudit("add_empresa", QString("Adicionou empresa '%1' (CNPJ: %2)").arg(nome, cnpj));
        QMessageBox::information(this, "Sucesso", QString("Empresa '%1' adicionada com sucesso!").arg(nome));
    } else {
        QMessageBox::critical(this, "Erro", "Falha ao adicionar empresa. Nome ou CNPJ já existe.");
    }
}

void EPIApp::updateEmpresa() {
    if (!checkAdmin("atualizar empresas")) return;

    QListWidgetItem *currentItem = empresasList->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "Erro", "Selecione uma empresa para atualizar!");
        return;
    }

    int empId = currentItem->data(Qt::UserRole).toInt();
    QString nome = empresaNome->text();
    QString cnpj = empresaCnpj->text();
    QString logadouro = empresaLogadouro->text();
    if (nome.isEmpty() || cnpj.isEmpty()) {
        QMessageBox::warning(this, "Erro", "Nome e CNPJ são obrigatórios!");
        return;
    }

    if (!confirmAction(QString("Atualizar empresa '%1' (ID: %2)?").arg(nome, QString::number(empId)))) {
        return;
    }

    if (dbManager->executeQuery(
            "UPDATE empresas SET nome=?, cnpj=?, logadouro=? WHERE id=?",
            {nome, cnpj, logadouro, empId})) {
        clearEmpresaForm();
        loadEmpresasList();
        loadEmpresas();
        logAudit("update_empresa", QString("Atualizou empresa '%1' (ID: %2)").arg(nome, QString::number(empId)));
        QMessageBox::information(this, "Sucesso", QString("Empresa '%1' atualizada com sucesso!").arg(nome));
    } else {
        QMessageBox::critical(this, "Erro", "Falha ao atualizar empresa. Nome ou CNPJ já existe.");
    }
}

void EPIApp::deleteEmpresa() {
    if (!checkAdmin("deletar empresas")) return;

    QList

WidgetItem *currentItem = empresasList->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "Erro", "Selecione uma empresa para deletar!");
        return;
    }

    int empId = currentItem->data(Qt::UserRole).toInt();
    QString nome = currentItem->text();
    if (!confirmAction(QString("Deletar empresa '%1' (ID: %2)?").arg(nome, QString::number(empId)))) {
        return;
    }

    if (dbManager->executeQuery("DELETE FROM empresas WHERE id=?", {empId})) {
        clearEmpresaForm();
        loadEmpresasList();
        loadEmpresas();
        logAudit("delete_empresa", QString("Deletou empresa '%1' (ID: %2)").arg(nome, QString::number(empId)));
        QMessageBox::information(this, "Sucesso", QString("Empresa '%1' deletada com sucesso!").arg(nome));
    } else {
        QMessageBox::critical(this, "Erro", "Falha ao deletar empresa. Empresa vinculada a colaboradores.");
    }
}

void EPIApp::filterItems() {
    QString searchText = searchInput->text().toLower();
    QString category = categoryFilter->currentText();
    QVariantList items;
    dbManager->executeQuery(
        "SELECT i.id, i.nome, i.ca, i.tamanho, i.marca, c.nome, i.quantidade, i.preco, i.estoque_minimo, i.fornecedor, i.data_adicao "
        "FROM itens i LEFT JOIN categorias c ON i.categoria_id = c.id", {}, true, &items);

    QList<QVariantList> filteredItems;
    for (const auto &item : items) {
        QString nome = item[1].toString().toLower();
        QString ca = item[2].toString().toLower();
        QString cat = item[5].toString();
        if ((searchText.isEmpty() || nome.contains(searchText) || ca.contains(searchText)) &&
            (category == "Todas as Categorias" || cat == category)) {
            filteredItems.append(item);
        }
    }

    itemsTable->setRowCount(filteredItems.size());
    for (int row = 0; row < filteredItems.size(); ++row) {
        const auto &item = filteredItems[row];
        for (int col = 0; col < item.size(); ++col) {
            itemsTable->setItem(row, col, new QTableWidgetItem(item[col].toString()));
            if (col == 6 && item[6].toInt() <= item[8].toInt()) {
                itemsTable->item(row, col)->setBackground(QColor(255, 200, 200));
            }
        }
    }
}

void EPIApp::loadItemDetails() {
    int currentRow = itemsTable->currentRow();
    if (currentRow < 0) return;

    QString itemId = itemsTable->item(currentRow, 0)->text();
    QVariantList item;
    dbManager->executeQuery(
        "SELECT nome, ca, tamanho, marca, categoria_id, quantidade, preco, estoque_minimo, fornecedor "
        "FROM itens WHERE id=?", {itemId}, true, &item);
    if (!item.isEmpty()) {
        itemName->setText(item[0].toString());
        itemCa->setText(item[1].toString());
        itemSize->setText(item[2].toString());
        itemBrand->setText(item[3].toString());
        int index = itemCategory->findData(item[4]);
        itemCategory->setCurrentIndex(index >= 0 ? index : 0);
        itemQuantity->setValue(item[5].toInt());
        itemPrice->setValue(item[6].toDouble());
        itemMinStock->setValue(item[7].toInt());
        itemSupplier->setText(item[8].toString());
    }
}

void EPIApp::onColaboradorSelected() {
    pendingWithdrawals.clear();
    pendingTable->setRowCount(0);
    movName->clear();
    movCa->clear();
    movSize->setCurrentIndex(0);
    movCategory->setCurrentIndex(0);
    movWithdrawQty->setValue(1);
    movValidDays->setValue(180);
}

void EPIApp::onMovNameChanged(const QString &text) {
    movSize->clear();
    movSize->addItem("Selecionar Tamanho");
    if (!text.isEmpty()) {
        QVariantList sizes;
        dbManager->executeQuery(
            "SELECT DISTINCT tamanho FROM itens WHERE (nome = ? OR ca = ?) AND quantidade > 0 AND tamanho IS NOT NULL ORDER BY tamanho",
            {text, text}, true, &sizes);
        for (const auto &size : sizes) {
            movSize->addItem(size[0].toString());
        }
    }
}

void EPIApp::onMovSizeSelected() {
    QString size = movSize->currentText();
    if (size == "Selecionar Tamanho") {
        movCa->clear();
        movCategory->setCurrentIndex(0);
        return;
    }

    QString itemNameText = movName->text();
    QVariantList item;
    dbManager->executeQuery(
        "SELECT i.ca, c.nome FROM itens i LEFT JOIN categorias c ON i.categoria_id = c.id "
        "WHERE (i.nome=? OR i.ca=?) AND i.tamanho=?", {itemNameText, itemNameText, size}, true, &item);
    if (!item.isEmpty()) {
        movCa->setText(item[0].toString());
        int index = movCategory->findText(item[1].toString());
        movCategory->setCurrentIndex(index >= 0 ? index : 0);
    }
}

void EPIApp::addToPending() {
    if (currentUserLevel != 2 && currentUserLevel != 3) {
        QMessageBox::warning(this, "Erro", "Apenas almoxarifes ou administradores podem registrar retiradas!");
        return;
    }

    int colaboradorId = colaboradorCombo->currentData().toInt();
    QString itemNameText = movName->text();
    QString size = movSize->currentText() == "Selecionar Tamanho" ? "" : movSize->currentText();
    int qty = movWithdrawQty->value();
    int validDays = movValidDays->value();

    if (!colaboradorId || itemNameText.isEmpty()) {
        QMessageBox::warning(this, "Erro", "Selecione um colaborador e digite o nome ou CA do EPI!");
        return;
    }

    QVariantList item;
    QString query = "SELECT id, nome, ca, tamanho, quantidade FROM itens WHERE (nome=? OR ca=?)";
    if (!size.isEmpty()) query += " AND tamanho=?";
    dbManager->executeQuery(query, size.isEmpty() ? QVariantList{itemNameText, itemNameText} : QVariantList{itemNameText, itemNameText, size}, true, &item);
    if (item.isEmpty()) {
        QMessageBox::warning(this, "Erro", "EPI não encontrado!");
        return;
    }

    int itemId = item[0].toInt();
    QString name = item[1].toString();
    QString ca = item[2].toString();
    QString itemSize = item[3].toString();
    int availableQty = item[4].toInt();
    if (qty > availableQty) {
        QMessageBox::warning(this, "Erro", QString("Quantidade solicitada (%1) excede o estoque disponível (%2)!").arg(qty).arg(availableQty));
        return;
    }

    if (!confirmAction(QString("Adicionar retirada de %1 '%2' (Tamanho: %3)?").arg(qty).arg(name).arg(itemSize.isEmpty() ? "N/A" : itemSize))) {
        return;
    }

    pendingWithdrawals.append({itemId, name, ca, itemSize, qty, validDays});
    updatePendingTable();
    logAudit("add_pending_withdrawal", QString("Adicionou retirada pendente: %1 '%2' (Tamanho: %3)").arg(qty).arg(name).arg(itemSize.isEmpty() ? "N/A" : itemSize));
}

void EPIApp::updatePendingTable() {
    pendingTable->setRowCount(pendingWithdrawals.size());
    for (int row = 0; row < pendingWithdrawals.size(); ++row) {
        const auto &item = pendingWithdrawals[row];
        pendingTable->setItem(row, 0, new QTableWidgetItem(item[1].toString()));
        pendingTable->setItem(row, 1, new QTableWidgetItem(item[2].toString()));
        pendingTable->setItem(row, 2, new QTableWidgetItem(item[3].toString()));
        pendingTable->setItem(row, 3, new QTableWidgetItem(QString::number(item[4].toInt())));
        QPushButton *removeBtn = new QPushButton("Remover");
        removeBtn->setProperty("delete", true);
        connect(removeBtn, &QPushButton::clicked, [this, row] { removePending(row); });
        pendingTable->setCellWidget(row, 4, removeBtn);
    }
}

void EPIApp::removePending(int row) {
    if (!confirmAction(QString("Remover retirada pendente: %1?").arg(pendingWithdrawals[row][1].toString()))) {
        return;
    }
    pendingWithdrawals.removeAt(row);
    updatePendingTable();
    logAudit("remove_pending_withdrawal", QString("Removeu retirada pendente: %1").arg(row));
}

void EPIApp::confirmPending() {
    if (pendingWithdrawals.isEmpty()) {
        QMessageBox::warning(this, "Erro", "Nenhuma retirada pendente para confirmar!");
        return;
    }

    int colaboradorId = colaboradorCombo->currentData().toInt();
    bool ok;
    QString password = QInputDialog::getText(this, "Verificação de Senha", "Digite a senha do colaborador:", QLineEdit::Password, &ok);
    if (!ok) return;

    QString hashedPassword = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
    QVariantList result;
    if (!dbManager->executeQuery(
            "SELECT id FROM usuarios WHERE id=? AND senha=?", {colaboradorId, hashedPassword}, true, &result)) {
        QMessageBox::warning(this, "Erro", "Senha incorreta!");
        return;
    }
    if (result.isEmpty()) {
        QMessageBox::warning(this, "Erro", "Senha incorreta!");
        return;
    }

    if (!confirmAction("Confirmar todas as retiradas pendentes?")) {
        return;
    }

    for (const auto &item : pendingWithdrawals) {
        int itemId = item[0].toInt();
        QString name = item[1].toString();
        QString size = item[3].toString();
        int qty = item[4].toInt();
        int validDays = item[5].toInt();
        if (dbManager->executeQuery(
                "UPDATE itens SET quantidade = quantidade - ? WHERE id=?", {qty, itemId})) {
            QString expiration = QDateTime::currentDateTime().addDays(validDays).toString("yyyy-MM-dd");
            dbManager->executeQuery(
                "INSERT INTO movimentacoes (item_id, alteracao_quantidade, data, motivo, colaborador_id, expiration_date) "
                "VALUES (?, ?, ?, ?, ?, ?)",
                {itemId, -qty, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"),
                 "Retirada por colaborador", colaboradorId, expiration});
            logAudit("confirm_withdrawal", QString("Confirmou retirada: %1 '%2' (Tamanho: %3)").arg(qty).arg(name).arg(size.isEmpty() ? "N/A" : size));
        } else {
            QMessageBox::critical(this, "Erro", QString("Falha ao confirmar retirada de '%1'!").arg(name));
            return;
        }
    }

    pendingWithdrawals.clear();
    pendingTable->setRowCount(0);
    loadItems();
    updateCompleters();
    // Note: updateDashboard() is not implemented here; see note below
    QMessageBox::information(this, "Sucesso", "Todas as retiradas foram confirmadas com sucesso!");
}

void EPIApp::onReturnColabSelected() {
    int colabId = returnColabCombo->currentData().toInt();
    if (!colabId) {
        withdrawnTable->setRowCount(0);
        return;
    }

    QVariantList withdrawn;
    dbManager->executeQuery(
        "SELECT i.id, i.nome, i.ca, i.tamanho, -SUM(m.alteracao_quantidade) as qty_owed "
        "FROM movimentacoes m JOIN itens i ON m.item_id = i.id "
        "WHERE m.colaborador_id = ? GROUP BY i.id HAVING SUM(m.alteracao_quantidade) < 0",
        {colabId}, true, &withdrawn);
    withdrawnTable->setRowCount(withdrawn.size());
    for (int row = 0; row < withdrawn.size(); ++row) {
        const auto &item = withdrawn[row];
        withdrawnTable->setItem(row, 0, new QTableWidgetItem(item[0].toString()));
        withdrawnTable->setItem(row, 1, new QTableWidgetItem(item[1].toString()));
        withdrawnTable->setItem(row, 2, new QTableWidgetItem(item[2].toString()));
        withdrawnTable->setItem(row, 3, new QTableWidgetItem(item[3].toString()));
        withdrawnTable->setItem(row, 4, new QTableWidgetItem(item[4].toString()));
        QSpinBox *qtyReturn = new QSpinBox;
        qtyReturn->setMinimum(0);
        qtyReturn->setMaximum(item[4].toInt());
        withdrawnTable->setCellWidget(row, 5, qtyReturn);
    }

    pendingReturns.clear();
    returnPendingTable->setRowCount(0);
}

void EPIApp::addReturnsToPending() {
    for (int row = 0; row < withdrawnTable->rowCount(); ++row) {
        QSpinBox *qtyReturn = qobject_cast<QSpinBox*>(withdrawnTable->cellWidget(row, 5));
        if (qtyReturn && qtyReturn->value() > 0) {
            int itemId = withdrawnTable->item(row, 0)->text().toInt();
            QString name = withdrawnTable->item(row, 1)->text();
            QString ca = withdrawnTable->item(row, 2)->text();
            QString size = withdrawnTable->item(row, 3)->text();
            int qty = qtyReturn->value();
            pendingReturns.append({itemId, name, ca, size, qty});
        }
    }
    updateReturnPendingTable();
}

void EPIApp::updateReturnPendingTable() {
    returnPendingTable->setRowCount(pendingReturns.size());
    for (int row = 0; row < pendingReturns.size(); ++row) {
        const auto &item = pendingReturns[row];
        returnPendingTable->setItem(row, 0, new QTableWidgetItem(item[1].toString()));
        returnPendingTable->setItem(row, 1, new QTableWidgetItem(item[2].toString()));
        returnPendingTable->setItem(row, 2, new QTableWidgetItem(item[3].toString()));
        returnPendingTable->setItem(row, 3, new QTableWidgetItem(QString::number(item[4].toInt())));
        QPushButton *removeBtn = new QPushButton("Remover");
        removeBtn->setProperty("delete", true);
        connect(removeBtn, &QPushButton::clicked, [this, row] { removeReturnPending(row); });
        returnPendingTable->setCellWidget(row, 4, removeBtn);
    }
}

void EPIApp::removeReturnPending(int row) {
    if (!confirmAction(QString("Remover devolução pendente: %1?").arg(pendingReturns[row][1].toString()))) {
        return;
    }
    pendingReturns.removeAt(row);
    updateReturnPendingTable();
    logAudit("remove_pending_return", QString("Removeu devolução pendente: %1").arg(row));
}

void EPIApp::confirmReturns() {
    if (pendingReturns.isEmpty()) {
        QMessageBox::warning(this, "Erro", "Nenhuma devolução pendente para confirmar!");
        return;
    }

    int colaboradorId = returnColabCombo->currentData().toInt();
    bool ok;
    QString password = QInputDialog::getText(this, "Verificação de Senha", "Digite a senha do colaborador:", QLineEdit::Password, &ok);
    if (!ok) return;

    QString hashedPassword = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
    QVariantList result;
    if (!dbManager->executeQuery(
            "SELECT id FROM usuarios WHERE id=? AND senha=?", {colaboradorId, hashedPassword}, true, &result)) {
        QMessageBox::warning(this, "Erro", "Senha incorreta!");
        return;
    }
    if (result.isEmpty()) {
        QMessageBox::warning(this, "Erro", "Senha incorreta!");
        return;
    }

    if (!confirmAction("Confirmar todas as devoluções pendentes?")) {
        return;
    }

    for (const auto &item : pendingReturns) {
        int itemId = item[0].toInt();
        QString name = item[1].toString();
        QString size = item[3].toString();
        int qty = item[4].toInt();
        if (dbManager->executeQuery(
                "UPDATE itens SET quantidade = quantidade + ? WHERE id=?", {qty, itemId})) {
            dbManager->executeQuery(
                "INSERT INTO movimentacoes (item_id, alteracao_quantidade, data, motivo, colaborador_id, expiration_date) "
                "VALUES (?, ?, ?, ?, ?, ?)",
                {itemId, qty, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"),
                 "Devolução por colaborador", colaboradorId, ""});
            logAudit("confirm_return", QString("Confirmou devolução: %1 '%2' (Tamanho: %3)").arg(qty).arg(name).arg(size.isEmpty() ? "N/A" : size));
        } else {
            QMessageBox::critical(this, "Erro", QString("Falha ao confirmar devolução de '%1'!").arg(name));
            return;
        }
    }

    pendingReturns.clear();
    returnPendingTable->setRowCount(0);
    loadItems();
    updateCompleters();
    onReturnColabSelected();
    QMessageBox::information(this, "Sucesso", "Todas as devoluções foram confirmadas com sucesso!");
}

void EPIApp::loadDelivered() {
    QString colabId = deliveredColab->currentData().toString();
    QString startDel = deliveredStartDel->text();
    QString endDel = deliveredEndDel->text();
    QString startExp = deliveredStartExp->text();
    QString endExp = deliveredEndExp->text();

    QString query = "SELECT u.nome_completo, i.nome, i.ca, i.tamanho, -m.alteracao_quantidade, m.data, m.expiration_date "
                    "FROM movimentacoes m "
                    "JOIN itens i ON m.item_id = i.id "
                    "JOIN usuarios u ON m.colaborador_id = u.id "
                    "WHERE m.alteracao_quantidade < 0";
    QVariantList params;
    if (colabId != "0") {
        query += " AND m.colaborador_id = ?";
        params.append(colabId);
    }
    if (!startDel.isEmpty()) {
        query += " AND m.data >= ?";
        params.append(startDel);
    }
    if (!endDel.isEmpty()) {
        query += " AND m.data <= ?";
        params.append(endDel);
    }
    if (!startExp.isEmpty()) {
        query += " AND m.expiration_date >= ?";
        params.append(startExp);
    }
    if (!endExp.isEmpty()) {
        query += " AND m.expiration_date <= ?";
        params.append(endExp);
    }

    QVariantList result;
    if (dbManager->executeQuery(query, params, true, &result)) {
        deliveredTable->setRowCount(result.size());
        for (int row = 0; row < result.size(); ++row) {
            const auto &item = result[row];
            for (int col = 0; col < item.size(); ++col) {
                deliveredTable->setItem(row, col, new QTableWidgetItem(item[col].toString()));
            }
        }
    } else {
        QMessageBox::critical(this, "Erro", "Falha ao carregar EPIs entregues!");
    }
}

void EPIApp::generateLowStockReport() {
    QVariantList items;
    dbManager->executeQuery(
        "SELECT i.nome, i.ca, i.tamanho, i.quantidade, i.estoque_minimo, c.nome "
        "FROM itens i LEFT JOIN categorias c ON i.categoria_id = c.id "
        "WHERE i.quantidade <= i.estoque_minimo", {}, true, &items);

    QString report = "<h2>Relatório de Estoque Baixo</h2>";
    report += "<table border='1' style='border-collapse: collapse; width: 100%;'>";
    report += "<tr><th>Nome</th><th>CA</th><th>Tamanho</th><th>Quantidade</th><th>Estoque Mínimo</th><th>Categoria</th></tr>";
    for (const auto &item : items) {
        report += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td></tr>")
                     .arg(item[0].toString(), item[1].toString(), item[2].toString(),
                          item[3].toString(), item[4].toString(), item[5].toString());
    }
    report += "</table>";
    reportDisplay->setHtml(report);
    logAudit("generate_low_stock_report", "Gerou relatório de estoque baixo");
}

void EPIApp::generateInventoryReport() {
    QVariantList items;
    dbManager->executeQuery(
        "SELECT i.nome, i.ca, i.tamanho, i.quantidade, i.estoque_minimo, i.preco, i.fornecedor, c.nome, i.data_adicao "
        "FROM itens i LEFT JOIN categorias c ON i.categoria_id = c.id", {}, true, &items);

    QString report = "<h2>Relatório Completo de EPIs</h2>";
    report += "<table border='1' style='border-collapse: collapse; width: 100%;'>";
    report += "<tr><th>Nome</th><th>CA</th><th>Tamanho</th><th>Quantidade</th><th>Estoque Mínimo</th><th>Preço</th><th>Fornecedor</th><th>Categoria</th><th>Data de Adição</th></tr>";
    for (const auto &item : items) {
        report += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td><td>%7</td><td>%8</td><td>%9</td></tr>")
                     .arg(item[0].toString(), item[1].toString(), item[2].toString(),
                          item[3].toString(), item[4].toString(), item[5].toString(),
                          item[7].toString(), item[8].toString());
    }
    report += "</table>";
    reportDisplay->setHtml(report);
    logAudit("generate_inventory_report", "Gerou relatório completo de EPIs");
}

void EPIApp::generateCategoryReport() {
    QVariantList categories;
    dbManager->executeQuery("SELECT id, nome FROM categorias", {}, true, &categories);

    QString report = "<h2>Relatório por Categoria</h2>";
    for (const auto &cat : categories) {
        int catId = cat[0].toInt();
        QString catName = cat[1].toString();
        QVariantList items;
        dbManager->executeQuery(
            "SELECT nome, ca, tamanho, quantidade, estoque_minimo FROM itens WHERE categoria_id=?",
            {catId}, true, &items);
        report += QString("<h3>%1</h3>").arg(catName);
        report += "<table border='1' style='border-collapse: collapse; width: 100%;'>";
        report += "<tr><th>Nome</th><th>CA</th><th>Tamanho</th><th>Quantidade</th><th>Estoque Mínimo</th></tr>";
        for (const auto &item : items) {
            report += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td></tr>")
                         .arg(item[0].toString(), item[1].toString(), item[2].toString(),
                              item[3].toString(), item[4].toString());
        }
        report += "</table>";
    }
    reportDisplay->setHtml(report);
    logAudit("generate_category_report", "Gerou relatório por categoria");
}

void EPIApp::showMostUsedGraph() {
    QString dateRange = getDateRange();
    QStringList conditions;
    QVariantList params;
    if (!dateRange.isEmpty()) {
        QStringList dates = dateRange.split(" AND ");
        conditions << "m.data >= ?" << "m.data <= ?";
        params << dates[0] << dates[1];
    }
    int colabId = collabFilter->currentData().toInt();
    if (colabId != 0) {
        conditions << "m.colaborador_id = ?";
        params << colabId;
    }
    QString query = "SELECT i.nome, COUNT(*) as count "
                    "FROM movimentacoes m JOIN itens i ON m.item_id = i.id "
                    "WHERE m.alteracao_quantidade < 0";
    if (!conditions.isEmpty()) {
        query += " AND " + conditions.join(" AND ");
    }
    query += " GROUP BY i.nome ORDER BY count DESC LIMIT 10";

    QVariantList result;
    dbManager->executeQuery(query, params, true, &result);

    QtCharts::QBarSeries *series = new QtCharts::QBarSeries;
    QtCharts::QBarSet *set = new QtCharts::QBarSet("Retiradas");
    QStringList categories;
    for (const auto &row : result) {
        set->append(row[1].toInt());
        categories << row[0].toString();
    }
    series->append(set);

    QtCharts::QChart *chart = new QtCharts::QChart;
    chart->addSeries(series);
    chart->setTitle("EPIs Mais Retirados");
    QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis;
    axisY->setTitleText("Número de Retiradas");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    QtCharts::QBarCategoryAxis *axisX = new QtCharts::QBarCategoryAxis;
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    chartView->setChart(chart);
    logAudit("show_most_used_graph", "Exibiu gráfico de EPIs mais usados");
}

void EPIApp::exportToCsv() {
    QString fileName = QFileDialog::getSaveFileName(this, "Exportar para CSV", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Erro", "Não foi possível abrir o arquivo para escrita!");
        return;
    }

    QTextStream out(&file);
    out << "Nome,CA,Tamanho,Quantidade,Estoque Mínimo,Preço,Fornecedor,Categoria,Data de Adição\n";
    QVariantList items;
    dbManager->executeQuery(
        "SELECT i.nome, i.ca, i.tamanho, i.quantidade, i.estoque_minimo, i.preco, i.fornecedor, c.nome, i.data_adicao "
        "FROM itens i LEFT JOIN categorias c ON i.categoria_id = c.id", {}, true, &items);
    for (const auto &item : items) {
        out << QString("\"%1\",\"%2\",\"%3\",%4,%5,%6,\"%7\",\"%8\",\"%9\"\n")
               .arg(item[0].toString(), item[1].toString(), item[2].toString(),
                    item[3].toString(), item[4].toString(), item[5].toString(),
                    item[6].toString(), item[7].toString(), item[8].toString());
    }
    file.close();
    logAudit("export_csv", QString("Exportou dados para CSV: %1").arg(fileName));
    QMessageBox::information(this, "Sucesso", "Dados exportados para CSV com sucesso!");
}

void EPIApp::exportToPdf() {
    QString fileName = QFileDialog::getSaveFileName(this, "Exportar para PDF", "", "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QPdfWriter writer(fileName);
    writer.setPageSize(QPageSize::A4);
    QPainter painter(&writer);
    painter.setRenderHint(QPainter::Antialiasing);

    QVariantList items;
    dbManager->executeQuery(
        "SELECT i.nome, i.ca, i.tamanho, i.quantidade, i.estoque_minimo, c.nome "
        "FROM itens i LEFT JOIN categorias c ON i.categoria_id = c.id "
        "WHERE i.quantidade <= i.estoque_minimo", {}, true, &items);

    painter.setFont(QFont("Segoe UI", 14, QFont::Bold));
    painter.drawText(100, 100, "Relatório de Estoque Baixo");
    painter.setFont(QFont("Segoe UI", 10));
    int y = 200;
    painter.drawText(100, y, "Nome");
    painter.drawText(300, y, "CA");
    painter.drawText(500, y, "Tamanho");
    painter.drawText(700, y, "Quantidade");
    painter.drawText(900, y, "Estoque Mínimo");
    painter.drawText(1100, y, "Categoria");
    y += 50;
    painter.drawLine(100, y, 1100, y);
    y += 20;

    for (const auto &item : items) {
        if (y > 7500) {
            writer.newPage();
            y = 100;
        }
        painter.drawText(100, y, item[0].toString());
        painter.drawText(300, y, item[1].toString());
        painter.drawText(500, y, item[2].toString());
        painter.drawText(700, y, item[3].toString());
        painter.drawText(900, y, item[4].toString());
        painter.drawText(1100, y, item[5].toString());
        y += 30;
    }

    painter.end();
    logAudit("export_pdf", QString("Exportou relatório para PDF: %1").arg(fileName));
    QMessageBox::information(this, "Sucesso", "Relatório exportado para PDF com sucesso!");
}

void EPIApp::logout() {
    if (QMessageBox::question(this, "Sair", "Deseja realmente sair do sistema?") == QMessageBox::Yes) {
        logAudit("logout", "Usuário realizou logout");
        close();
    }
}

void EPIApp::refreshAllData() {
    loadUsers();
    loadItems();
    loadCategories();
    loadColaboradores();
    loadDeliveredColab();
    loadColabFilter();
    loadEmpresas();
    loadEmpresasList();
    updateCompleters();
    logAudit("refresh_data", "Atualizou todos os dados");
}

void EPIApp::loadUsers() {
    if (!checkAdmin("carregar usuários")) return;

    QVariantList users;
    dbManager->executeQuery(
        "SELECT u.id, u.nome_completo, u.matricula, u.cpf, u.level, e.nome "
        "FROM usuarios u LEFT JOIN empresas e ON u.empresa_id = e.id", {}, true, &users);
    usersTable->setRowCount(users.size());
    for (int row = 0; row < users.size(); ++row) {
        const auto &user = users[row];
        for (int col = 0; col < user.size(); ++col) {
            usersTable->setItem(row, col, new QTableWidgetItem(user[col].toString()));
        }
    }
}

void EPIApp::loadItems() {
    QVariantList items;
    dbManager->executeQuery(
        "SELECT i.id, i.nome, i.ca, i.tamanho, i.marca, c.nome, i.quantidade, i.preco, i.estoque_minimo, i.fornecedor, i.data_adicao "
        "FROM itens i LEFT JOIN categorias c ON i.categoria_id = c.id", {}, true, &items);
    itemsTable->setRowCount(items.size());
    for (int row = 0; row < items.size(); ++row) {
        const auto &item = items[row];
        for (int col = 0; col < item.size(); ++col) {
            itemsTable->setItem(row, col, new QTableWidgetItem(item[col].toString()));
            if (col == 6 && item[6].toInt() <= item[8].toInt()) {
                itemsTable->item(row, col)->setBackground(QColor(255, 200, 200));
            }
        }
    }
}

void EPIApp::loadCategories() {
    QVariantList categories;
    dbManager->executeQuery("SELECT id, nome FROM categorias", {}, true, &categories);
    categoriesList->clear();
    itemCategory->clear();
    itemCategory->addItem("Selecionar Categoria", 0);
    movCategory->clear();
    movCategory->addItem("Selecionar Categoria");
    categoryFilter->clear();
    categoryFilter->addItem("Todas as Categorias");
    for (const auto &cat : categories) {
        categoriesList->addItem(cat[1].toString())->setData(Qt::UserRole, cat[0]);
        itemCategory->addItem(cat[1].toString(), cat[0]);
        movCategory->addItem(cat[1].toString());
        categoryFilter->addItem(cat[1].toString());
    }
}

void EPIApp::loadColaboradores() {
    QVariantList users;
    dbManager->executeQuery("SELECT id, nome_completo FROM usuarios WHERE level IN (1, 2)", {}, true, &users);
    colaboradorCombo->clear();
    returnColabCombo->clear();
    colaboradorCombo->addItem("Selecionar Colaborador", 0);
    returnColabCombo->addItem("Selecionar Colaborador", 0);
    for (const auto &user : users) {
        colaboradorCombo->addItem(user[1].toString(), user[0]);
        returnColabCombo->addItem(user[1].toString(), user[0]);
    }
}

void EPIApp::loadDeliveredColab() {
    QVariantList users;
    dbManager->executeQuery("SELECT id, nome_completo FROM usuarios", {}, true, &users);
    deliveredColab->clear();
    deliveredColab->addItem("Todos", 0);
    for (const auto &user : users) {
        deliveredColab->addItem(user[1].toString(), user[0]);
    }
}

void EPIApp::loadColabFilter() {
    QVariantList users;
    dbManager->executeQuery("SELECT id, nome_completo FROM usuarios", {}, true, &users);
    collabFilter->clear();
    collabFilter->addItem("Selecionar Colaborador", 0);
    for (const auto &user : users) {
        collabFilter->addItem(user[1].toString(), user[0]);
    }
}

void EPIApp::loadEmpresas() {
    QVariantList empresas;
    dbManager->executeQuery("SELECT id, nome FROM empresas", {}, true, &empresas);
    userEmpresa->clear();
    userEmpresa->addItem("Selecionar Empresa", 0);
    for (const auto &emp : empresas) {
        userEmpresa->addItem(emp[1].toString(), emp[0]);
    }
}

void EPIApp::loadEmpresasList() {
    QVariantList empresas;
    dbManager->executeQuery("SELECT id, nome FROM empresas", {}, true, &empresas);
    empresasList->clear();
    for (const auto &emp : empresas) {
        empresasList->addItem(emp[1].toString())->setData(Qt::UserRole, emp[0]);
    }
}

void EPIApp::updateCompleters() {
    QStringList names, cas;
    QVariantList items;
    dbManager->executeQuery("SELECT nome, ca FROM itens WHERE quantidade > 0", {}, true, &items);
    for (const auto &item : items) {
        names << item[0].toString();
        cas << item[1].toString();
    }
    QCompleter *nameCompleter = new QCompleter(names, this);
    nameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    movName->setCompleter(nameCompleter);
    QCompleter *caCompleter = new QCompleter(cas, this);
    caCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    movName->setCompleter(caCompleter);
}

void EPIApp::loadCategoryDetails(QListWidgetItem *item) {
    int catId = item->data(Qt::UserRole).toInt();
    QVariantList result;
    dbManager->executeQuery("SELECT nome, descricao FROM categorias WHERE id=?", {catId}, true, &result);
    if (!result.isEmpty()) {
        categoryName->setText(result[0][0].toString());
        categoryDescription->setPlainText(result[0][1].toString());
    }
}

void EPIApp::loadEmpresaDetails(QListWidgetItem *item) {
    int empId = item->data(Qt::UserRole).toInt();
    QVariantList result;
    dbManager->executeQuery("SELECT nome, cnpj, logadouro FROM empresas WHERE id=?", {empId}, true, &result);
    if (!result.isEmpty()) {
        empresaNome->setText(result[0][0].toString());
        empresaCnpj->setText(result[0][1].toString());
        empresaLogadouro->setText(result[0][2].toString());
    }
}

void EPIApp::logAudit(const QString &action, const QString &details) {
    dbManager->executeQuery(
        "INSERT INTO audit_logs (user_id, action, details, timestamp) VALUES (?, ?, ?, ?)",
        {currentUserId, action, details, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")});
}

void EPIApp::handleError(const QString &action, const QString &error, const QString &message) {
    logAudit(action + "_error", error);
    QMessageBox::critical(this, "Erro", message);
}

bool EPIApp::confirmAction(const QString &message) {
    return QMessageBox::question(this, "Confirmação", message) == QMessageBox::Yes;
}

QString EPIApp::getDateRange() {
    QString period = timeFilter->currentText();
    QDateTime end = QDateTime::currentDateTime();
    QDateTime start;

    if (period == "1 Dia") {
        start = end.addDays(-1);
    } else if (period == "1 Semana") {
        start = end.addDays(-7);
    } else if (period == "15 Dias") {
        start = end.addDays(-15);
    } else if (period == "1 Mês") {
        start = end.addMonths(-1);
    } else if (period == "3 Meses") {
        start = end.addMonths(-3);
    } else if (period == "6 Meses") {
        start = end.addMonths(-6);
    } else if (period == "1 Ano") {
        start = end.addYears(-1);
    } else if (period == "5 Anos") {
        start = end.addYears(-5);
    } else if (period == "10 Anos") {
        start = end.addYears(-10);
    } else {
        return "";
    }

    return QString("%1 AND %2").arg(start.toString("yyyy-MM-dd"), end.toString("yyyy-MM-dd"));
}

void EPIApp::clearUserForm() {
    userNomeCompleto->clear();
    userMatricula->clear();
    userCpf->clear();
    userSenha->clear();
    userLevel->setCurrentIndex(0);
    userEmpresa->setCurrentIndex(0);
}

void EPIApp::clearItemForm() {
    itemName->clear();
    itemCa->clear();
    itemSize->clear();
    itemBrand->clear();
    itemCategory->setCurrentIndex(0);
    itemQuantity->setValue(0);
    itemPrice->setValue(0.0);
    itemMinStock->setValue(0);
    itemSupplier->clear();
}

void EPIApp::clearCategoryForm() {
    categoryName->clear();
    categoryDescription->clear();
}

void EPIApp::clearEmpresaForm() {
    empresaNome->clear();
    empresaCnpj->clear();
    empresaLogadouro->clear();
}
