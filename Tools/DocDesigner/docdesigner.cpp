#include <QMainWindow>
#include <QComboBox>
#include <QAction>
#include <QStatusBar>
#include <QPainter>
#include <QTextDocumentWriter>
#include <textedit.h>

#include "docdesigner.h"

DocDesigner::DocDesigner(QWidget *parent)
    : HtmlTextEditor(parent),
      m_current_chapter_level_1(0),
      m_current_chapter_level_2(0),
      m_current_chapter_level_3(0),
      m_default_font("DejaVu Sans", 10, false, false, false, QColor(Qt::black), Qt::AlignLeft),
      m_current_font("DejaVu Sans", 10, false, false, false, QColor(Qt::black), Qt::AlignLeft)
{
    // bug d'affichage dans certains cas si cette séquence n'est pas appelée
    show();
    hide();

    setDefaultFont();
}

// ________________________________________________________
QString DocDesigner::getDefaultFamilyFont()
{
    return m_default_font.m_family;
}

// ________________________________________________________
void DocDesigner::setDefaultFont()
{
    textFamily(m_default_font.m_family);
    textSize(m_default_font.m_size);
    textBold(m_default_font.m_bold);
    textItalic(m_default_font.m_italic);
    textUnderline(m_default_font.m_underline);
    textColor(m_default_font.m_color);
    textAlign(m_default_font.m_align);
    textStyle(QTextListFormat::ListStyleUndefined);

    m_current_font = m_default_font;
}

// ________________________________________________________
void DocDesigner::setDefaultFont(const DocFont& font)
{
    m_default_font.m_family     = font.m_family;
    m_default_font.m_size       = font.m_size;
    m_default_font.m_bold       = font.m_bold;
    m_default_font.m_italic     = font.m_italic;
    m_default_font.m_underline  = font.m_underline;
    m_default_font.m_color      = font.m_color;
    m_default_font.m_align      = font.m_align;
    setDefaultFont();
}

// ________________________________________________________
void DocDesigner::setFont(const DocFont& font)
{
    if (font.m_family != "") textFamily(font.m_family);
    textSize(font.m_size);
    textBold(font.m_bold);
    textItalic(font.m_italic);
    textUnderline(font.m_underline);
    textColor(font.m_color);
    textAlign(font.m_align);

    if (font.m_family != "") m_current_font.m_family = font.m_family;
    m_current_font.m_size       = font.m_size;
    m_current_font.m_bold       = font.m_bold;
    m_current_font.m_italic     = font.m_italic;
    m_current_font.m_underline  = font.m_underline;
    m_current_font.m_color      = font.m_color;
    m_current_font.m_align      = font.m_align;
}

// ________________________________________________________
QString DocDesigner::getFormatedText(const QString& text, const DocFont& font, bool final_crlf)
{
    DocDesigner doc;
    doc.setFont(font);
    doc.appendText(text, final_crlf);
    return doc.getHtml();
}

// ________________________________________________________
void DocDesigner::setAlign(Qt::Alignment align)
{
  textAlign(align);
  m_current_font.m_align = align;
}

// ________________________________________________________
void DocDesigner::appendText(QString text, bool final_crlf)
{
    textEdit->insertPlainText(text);
    if (final_crlf) textEdit->insertPlainText("\n");
}

// ________________________________________________________
void DocDesigner::appendText(QString text, const DocFont& font, bool final_crlf)
{
    textEdit->insertHtml(getFormatedText(text, font));
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
     setFont(DocFont(m_default_font.m_family,
                    24,     // size
                    true,   // bold
                    false,  // italic
                    true,   // underline
                    QColor("darkBlue"),
                    Qt::AlignHCenter));
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

    setDefaultFont();
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
        textUnderline(false);
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
        textUnderline(false);
        textEdit->insertPlainText("         ");   //indentation
        textBold(true);
        textUnderline(true);
        textSize("11");
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
//! BUG : si la cellule est composée de texte brut (et pas de html),
//!   la police de caractère de cette cellule est une police par défaut.
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
    setFont(m_current_font);  // Restitue la dernière police de caractère utilisée qui a été perdue lors de l'insertion du tableau
}

// ______________________________________________________
void DocDesigner::appendPicture(const QImage& image, double scale, bool final_crlf)
{
    textEdit->dropImage(image, "JPG", scale);
    if (final_crlf) textEdit->insertPlainText("\n");
    setFont(m_current_font);  // Restitue la dernière police de caractère utilisée qui a été perdue lors de l'insertion de l'image
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

// ______________________________________________________
void DocDesigner::appendDoc(DocDesigner *doc)
{
    textEdit->insertHtml(doc->getHtml());
}

// ______________________________________________________
void DocDesigner::appendHtml(QString html)
{
    textEdit->insertHtml(html);
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

// ________________________________________________________
void DocDesigner::exportToHtml(QString pathfilename)
{
    QTextDocumentWriter writer(pathfilename);
    if (! (pathfilename.endsWith(".htm") || (pathfilename.endsWith(".html")))) {
            pathfilename.append(".html");
    }
    writer.setFormat("html");
    writer.write(textEdit->document());
}

// ________________________________________________________
//! BUG : les images ne sont pas présentes dans le document ODF généré
void DocDesigner::exportToOdf(QString pathfilename)
{
    QTextDocumentWriter writer(pathfilename);
    if (!pathfilename.endsWith(".odf")) {
        pathfilename.append(".odf");
    }
    writer.setFormat("odf");
    writer.write(textEdit->document());
}
