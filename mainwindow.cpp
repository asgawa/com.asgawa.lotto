#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMetaMethod>
#include <QRandomGenerator>
#include <QScrollBar>
#include <QTimer>
#include <QFile>
#include <QDateTime>
#include <QClipboard>

#include <thread>

static const qint8 LOTTO_MIN = 1;
static const qint8 LOTTO_MAX = 45;

inline QString qintToString(qint8 i) {
    return QString("%1").arg(QString::number(i), 2, u'0');
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , filledLotto(0)
    , blankItem(0)
{
    ui->setupUi(this);

    setWindowTitle("로또 생성기");

    for (int i=LOTTO_MIN; i<=LOTTO_MAX; ++i) {
        lotto.push_back(i);
    }

    connect(ui->btnGenerate, &QPushButton::clicked, this, &MainWindow::onClickedGenerate);
    connect(ui->btnGenerate5, &QPushButton::clicked, this, &MainWindow::onClickedGenerate5);
    connect(ui->btnScreenshot, &QPushButton::clicked, this, &MainWindow::onClickedScreenshot);
    connect(ui->btnClear, &QPushButton::clicked, this, &MainWindow::onClickedClear);

    auto model = ui->lwOutput->model();
    connect(model, &QAbstractItemModel::rowsInserted, this, &MainWindow::onListItemChanged);
#if 0
    ui->btnGenerate5->setAutoFillBackground(true);

    QPalette p = ui->btnGenerate5->palette();
    p.setColor(QPalette::Button, QColor(0xFF, 0x33, 0xFF));
    ui->btnGenerate5->setPalette(p);
    ui->btnGenerate5->update();
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::generate()
{
    std::vector<qint8> l;

    while (l.size() < 6) {
        const auto g = QRandomGenerator::global()->bounded(LOTTO_MIN, LOTTO_MAX + 1);
        if (std::find(l.begin(), l.end(), g) == l.end()) {
            l.push_back(g);
        }
        // result.append(QString::number(QRandomGenerator::global()->bounded(LOTTO_MIN, LOTTO_MAX + 1)));
        // result.append(" ");
    }
    std::sort(l.begin(), l.end());

    QString result = QString("%1 %2 %3 %4 %5 %6").
                     arg(qintToString(l[0]), qintToString(l[1]), qintToString(l[2]), qintToString(l[3]), qintToString(l[4]), qintToString(l[5]));

    return result;
}

void MainWindow::onClickedGenerate(const bool setToEnd)
{
    const auto numbers = generate();
    if (!numbers.isEmpty()) {
        ++filledLotto;
        ui->lwOutput->addItem(numbers);
    }

    std::thread t([&, numbers]{
        saveToFile(numbers);
    });
    t.detach();

    if (setToEnd) {
        QTimer::singleShot(1, this, [&]{
            ui->lwOutput->verticalScrollBar()->setValue(ui->lwOutput->verticalScrollBar()->maximum());
        });
    }
}

void MainWindow::onClickedGenerate5()
{
    for (int i=0; i<5; ++i) {
        onClickedGenerate(false);
    }
    QTimer::singleShot(1, this, [&]{
        ui->lwOutput->verticalScrollBar()->setValue(ui->lwOutput->verticalScrollBar()->maximum());
    });
}

void MainWindow::onClickedScreenshot()
{
    QPixmap outPixmap = ui->lwOutput->grab();
    QImage outImage = outPixmap.toImage();
    QApplication::clipboard()->setImage(outImage);
}

void MainWindow::onClickedClear()
{
    ui->lwOutput->clear();
    filledLotto = 0;
    blankItem = 0;
}

void MainWindow::onListItemChanged(const QModelIndex & parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);

    const auto itemCount = ui->lwOutput->count() - blankItem;
    if ((itemCount % 5) == 0) {
        addBlankLine();
    }
}

void MainWindow::addBlankLine()
{
    ui->lwOutput->addItem("");
    ++blankItem;
}

void MainWindow::saveToFile(const QString &data)
{
    std::lock_guard<std::mutex> lg(mtx);
    QFile f("lotto.txt");
    if (f.open(QIODevice::ReadWrite | QIODevice::Append)) {
        const auto currentDateTime = QDateTime::currentDateTime();
        const auto dateTime = currentDateTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
        const QString msg = QString("[%1] %2").arg(dateTime, data);
        f.write(msg.toLocal8Bit() + "\n");
        f.close();
    } else {
        qDebug("Cannot open %s", qPrintable(f.fileName()));
    }
}
