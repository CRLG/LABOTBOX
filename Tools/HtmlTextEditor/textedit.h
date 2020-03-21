#ifndef _TEXT_EDIT_H_
#define _TEXT_EDIT_H_

#include <QTextEdit>
#include <QMimeData>
#include <QImage>

class TextEdit : public QTextEdit
{
    Q_OBJECT
public:
    TextEdit(QWidget *parent);

protected:
    bool canInsertFromMimeData(const QMimeData *source) const;
    void insertFromMimeData(const QMimeData *source);
    QMimeData *createMimeDataFromSelection() const;

public slots :
    void dropImage(const QImage& image, const QString& format, double scale_factor=1.0);
};

#endif
