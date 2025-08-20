#include "DatabaseManager.h"
#include <QSqlError>
#include <QCryptographicHash>
#include <QDebug>

DatabaseManager::DatabaseManager(const QString &db_name) {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(db_name);
    if (!db.open()) {
        qDebug() << "Database Error:" << db.lastError().text();
        return;
    }
    initDatabase();
}

DatabaseManager::~DatabaseManager() {
    if (db.isOpen()) {
        db.close();
    }
}

void DatabaseManager::initDatabase() {
    QStringList queries = {
        "CREATE TABLE IF NOT EXISTS usuarios ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "nome_usuario TEXT UNIQUE, "
        "senha TEXT, "
        "level INTEGER, "
        "nome_completo TEXT, "
        "matricula TEXT UNIQUE, "
        "cpf TEXT UNIQUE, "
        "empresa_id INTEGER, "
        "FOREIGN KEY (empresa_id) REFERENCES empresas (id))",
        "CREATE TABLE IF NOT EXISTS empresas ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "nome TEXT UNIQUE, "
        "cnpj TEXT UNIQUE, "
        "logadouro TEXT)",
        "CREATE TABLE IF NOT EXISTS categorias ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "nome TEXT UNIQUE, "
        "descricao TEXT)",
        "CREATE TABLE IF NOT EXISTS itens ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "nome TEXT, "
        "categoria_id INTEGER, "
        "quantidade INTEGER, "
        "preco REAL, "
        "estoque_minimo INTEGER, "
        "fornecedor TEXT, "
        "data_adicao TEXT, "
        "ca TEXT, "
        "tamanho TEXT, "
        "marca TEXT, "
        "FOREIGN KEY (categoria_id) REFERENCES categorias (id))",
        "CREATE TABLE IF NOT EXISTS movimentacoes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "item_id INTEGER, "
        "alteracao_quantidade INTEGER, "
        "data TEXT, "
        "motivo TEXT, "
        "colaborador_id INTEGER, "
        "expiration_date TEXT, "
        "FOREIGN KEY (item_id) REFERENCES itens (id), "
        "FOREIGN KEY (colaborador_id) REFERENCES usuarios (id))",
        "CREATE TABLE IF NOT EXISTS audit_logs ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "user_id INTEGER, "
        "action TEXT, "
        "details TEXT, "
        "timestamp TEXT, "
        "FOREIGN KEY (user_id) REFERENCES usuarios (id))",
        "CREATE INDEX IF NOT EXISTS idx_itens_nome ON itens(nome)",
        "CREATE INDEX IF NOT EXISTS idx_itens_ca ON itens(ca)",
        "CREATE INDEX IF NOT EXISTS idx_movimentacoes_data ON movimentacoes(data)"
    };

    QSqlQuery query;
    for (const auto &q : queries) {
        if (!query.exec(q)) {
            qDebug() << "Database Init Error:" << query.lastError().text();
        }
    }

    // Insert default admin user
    QString adminPassword = QString(QCryptographicHash::hash("admin", QCryptographicHash::Sha256).toHex());
    query.prepare("INSERT OR IGNORE INTO usuarios (id, nome_usuario, senha, level, nome_completo, matricula, cpf, empresa_id) "
                  "VALUES (1, 'admin', :senha, 3, 'Administrador', 'ADMIN001', '000.000.000-00', NULL)");
    query.bindValue(":senha", adminPassword);
    if (!query.exec()) {
        qDebug() << "Admin Insert Error:" << query.lastError().text();
    }

    // Insert default categories
    QStringList categories = {"Luvas", "Óculos", "Capacete", "Botina", "Abafador", "Máscara/Respirador", "Cinto de Segurança"};
    for (const auto &cat : categories) {
        query.prepare("INSERT OR IGNORE INTO categorias (nome) VALUES (:nome)");
        query.bindValue(":nome", cat);
        if (!query.exec()) {
            qDebug() << "Category Insert Error:" << query.lastError().text();
        }
    }
}

bool DatabaseManager::executeQuery(const QString &queryStr, const QVariantList &params, bool fetch, QVariantList *result) {
    QSqlQuery query;
    query.prepare(queryStr);
    for (int i = 0; i < params.size(); ++i) {
        query.bindValue(i, params[i]);
    }

    if (!query.exec()) {
        qDebug() << "Query Error:" << query.lastError().text();
        return false;
    }

    if (fetch && result) {
        result->clear();
        while (query.next()) {
            QVariantList row;
            for (int i = 0; i < query.record().count(); ++i) {
                row.append(query.value(i));
            }
            result->append(row);
        }
    }
    return true;
}
