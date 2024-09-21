#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>

#include <vector>
#include <mutex>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    std::vector<qint8> lotto;
    int filledLotto;
    int blankItem;
    std::mutex mtx;

    QString generate();

private slots:
    void onClickedGenerate(const bool setToEnd = true);
    void onClickedGenerate5();
    void onClickedScreenshot();
    void onClickedClear();
    void onListItemChanged(const QModelIndex & parent, int start, int end);

    void addBlankLine();

    void saveToFile(const QString& data);
};
#endif // MAINWINDOW_H
