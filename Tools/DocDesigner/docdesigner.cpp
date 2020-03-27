#include <QMainWindow>
#include <QComboBox>
#include <QAction>
#include <QStatusBar>
#include <QPainter>
#include <textedit.h>

#include "docdesigner.h"

DocDesigner::DocDesigner(QWidget *parent)
    : HtmlTextEditor(parent),
      m_current_chapter_level_1(0),
      m_current_chapter_level_2(0),
      m_current_chapter_level_3(0),
      m_default_font_family("DejaVu Sans"),
      m_default_font_size(10),
      m_default_font_bold(false),
      m_default_font_italic(false),
      m_default_font_underline(false),
      m_default_font_color(QColor(Qt::black)),
      m_default_font_align(Qt::AlignLeft)
{
    // bug d'affichage dans certains cas si cette séquence n'est pas appelée
    show();
    hide();

    setDefaultFont();
}

// ________________________________________________________
QString DocDesigner::getDefaultFamilyFont()
{
    return m_default_font_family;
}

// ________________________________________________________
void DocDesigner::setDefaultFont()
{
    textAlign(m_default_font_align);
    textStyle(QTextListFormat::ListStyleUndefined);
    textBold(m_default_font_bold);
    textUnderline(m_default_font_underline);
    textItalic(m_default_font_italic);
    textSize(m_default_font_size);
    textFamily(getDefaultFamilyFont());
    textColor(m_default_font_color);
}

void DocDesigner::setDefaultFont(QString family, qreal size, bool bold, bool italic, bool underline, QColor color, Qt::Alignment align)
{
    m_default_font_family       = family;
    m_default_font_size         = size;
    m_default_font_bold         = bold;
    m_default_font_italic       = italic;
    m_default_font_underline    = underline;
    m_default_font_color        = color;
    m_default_font_align        = align;
    setDefaultFont();
}


// ________________________________________________________
void DocDesigner::setFont(QString family, qreal size, bool bold, bool italic, bool underline, QColor color, Qt::Alignment align)
{
    textAlign(align);
    textBold(bold);
    textUnderline(underline);
    textItalic(italic);
    textSize(QString::number(size));
    textFamily(family);
    textColor(color);
}

// ________________________________________________________
void DocDesigner::setAlign(Qt::Alignment align)
{
  textAlign(align);
}

// ________________________________________________________
void DocDesigner::appendText(QString text, bool final_crlf)
{
    textEdit->insertPlainText(text);
    if (final_crlf) textEdit->insertPlainText("\n");
}

// ________________________________________________________
void DocDesigner::appendCRLF(unsigned int number)
{
    for (unsigned int i=0; i<number; i++) {
        textEdit->insertPlainText("\n");
    }
}

// ________________________________________________________
void DocDesigner::appendDocTitle(QString text)
{
    setFont(m_default_font_family,
            24,     // size
            true,   // bold
            false,  // italic
            true,   // underline
            QColor("darkBlue"),
            Qt::AlignHCenter);
    textEdit->insertPlainText(text);
    if (!text.endsWith("\n")) {
         textEdit->insertPlainText("\n");
    }
    setDefaultFont();
}

// ________________________________________________________
void DocDesigner::appendChapterTitle(int level, QString title, bool final_crlf)
{
    QString txt;

    setFont(m_default_font_family,
            10,     // size
            false,   // bold
            false,  // italic
            false,   // underline
            QColor("black"),
            Qt::AlignLeft);
    switch(level) {
    // 1. Titre Niveau 1
    case 0:
    case 1:
        textBold(true);
        textUnderline(true);
        textSize("14");
        m_current_chapter_level_1++;
        m_current_chapter_level_2 = 0;
        m_current_chapter_level_3 = 0;

        txt += QString::number(m_current_chapter_level_1);
        txt += ". ";
        txt += title;
        txt += "\n";
        textEdit->insertPlainText(txt);
        break;
    // 1.1 Titre Niveau 2
    case 2:
        textEdit->insertPlainText("    ");   //indentation
        textBold(true);
        textUnderline(true);
        textSize("12");
        m_current_chapter_level_2++;
        m_current_chapter_level_3 = 0;
        txt += QString::number(m_current_chapter_level_1);
        txt += ".";
        txt += QString::number(m_current_chapter_level_2);
        txt += ". ";
        txt += title;
        txt += "\n";
        textEdit->insertPlainText(txt);
        break;
    // 1.1.1 Titre Niveau 3
    case 3:
        textEdit->insertPlainText("         ");   //indentation
        textBold(true);
        textUnderline(true);
        textSize("10");
        m_current_chapter_level_3++;
        txt += QString::number(m_current_chapter_level_1);
        txt += ".";
        txt += QString::number(m_current_chapter_level_2);
        txt += ".";
        txt += QString::number(m_current_chapter_level_3);
        txt += ". ";
        txt += title;
        txt += "\n";
        textEdit->insertPlainText(txt);
        break;
     default :
        txt += title;
        txt += "\n";
        textEdit->insertPlainText(txt);
        break;
    }
    if (final_crlf) appendCRLF();
    setDefaultFont();
}

// ________________________________________________________
void DocDesigner::appendBulletList(QStringList lst, QTextListFormat::Style style)
{
  textStyle(style);
  for (int i=0; i<lst.size(); i++) {
      appendText(lst.at(i));
  }
  textStyle(QTextListFormat::ListStyleUndefined);
}

// ______________________________________________________
void DocDesigner::appendTable(QVector<QStringList> rows)
{
    QString html;

    if (rows.size() == 0) return;

    html += "<table border =\"10\">";
    for (int row=0; row<rows.size(); row++) {
        QStringList row_str = rows.at(row);
        html += "<tr>";
        for (int column=0; column<row_str.size(); column++) {
            html += "<td>";
            html += row_str.at(column);
            html += "</td>";
        }
        html += "</tr>";
    }
    html += "</table>";
    textEdit->insertHtml(html);
}

// ______________________________________________________
void DocDesigner::appendPicture(const QImage& image, double scale, bool final_crlf)
{
    textEdit->dropImage(image, "JPG", scale);
    if (final_crlf) textEdit->insertPlainText("\n");;
}

// ______________________________________________________
void DocDesigner::appendPicture(QString pathfilename, double scale, bool final_crlf)
{
    appendPicture(QImage(pathfilename), scale, final_crlf);
}

// ______________________________________________________
void DocDesigner::appendWidget(QWidget* wdgt, double scale, bool final_crlf)
{
    QImage image(wdgt->size(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    wdgt->render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
    appendPicture(image, scale, final_crlf);
}

// ________________________________________________________
QString DocDesigner::getHtml()
{
    return textEdit->toHtml();
}

// ________________________________________________________
void DocDesigner::exportToPdf(QString pathfilename)
{
    saveToPdf(pathfilename);
}


