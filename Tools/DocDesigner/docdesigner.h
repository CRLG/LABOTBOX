#ifndef _DOC_DESIGNER_H_
#define _DOC_DESIGNER_H_

#include "htmltexteditor.h"

/*
  Classe d'aide à la création de documents.
  Cette classe permet de créer des titres, des chapitres, des tableaux, du texte, du texte sur puces, des images.
  La police du texte peut être sélectionnée (taille, couleur, gras, ...), l'alignement des objets également.
  Le document est construit au fur et à mesure et peut être visualisé, exporté en html, exporté en PDF.

  Les images peuvent provenir soit de fichiers, soit de widgets Qt. Il est ainsi possible d'inclure dans le document
   la copie d'écran d'une fenêtre affichée à l'utilisateur.

  - Cas particulier des tableaux
  Un tableau est vu comme un QVector de QStringList où :
    * Chaque élément du QStringList représente une colonne
    * Chaque élément du QVector représente une ligne
  Les cellules peuvent être remplies avec du texte simple.
  Il est possible d'insérer dans une cellule un objet DocDesigner, contenant lui même des images, des puces, du texte formaté, ...
  Cela permet de créer des cellules complexes.

  - Cas des images
  Les images sont converties et embarquées directement dans le document (pas de référence à un fichier externe)

  - Exemple d'utilisation :

  DocDesigner m_doc_designer;
  m_doc_designer.show();

  m_doc_designer.setAlign(Qt::AlignHCenter);
  m_doc_designer.appendPicture(":/icons/bloc_note.png", 2.5);

  m_doc_designer.appendDocTitle("Titre du document");
  m_doc_designer.setAlign(Qt::AlignRight);
  m_doc_designer.appendText("Ecrit par Toto, le xx/xx/xx");


  m_doc_designer.appendChapterTitle(1, "Nom du chapitre 1");
  m_doc_designer.appendText("Du texte juste en dessous du Chapitre 1");

  m_doc_designer.appendCRLF();
  m_doc_designer.appendChapterTitle(2, "Nom du chapitre 1.1");
  m_doc_designer.setFont(DocFont("Courier", 14, true, true, true, QColor(Qt::darkRed)));
  m_doc_designer.appendText("Du texte en rouge juste en dessous du Chapitre 1.1");

  m_doc_designer.appendCRLF();
  m_doc_designer.appendChapterTitle(2, "Nom du chapitre 1.2");
  m_doc_designer.setDefaultFont();
  m_doc_designer.appendText("Du texte normal juste en dessous du Chapitre 1.2");
  m_doc_designer.appendCRLF();
  m_doc_designer.appendText("Une ligne vide et du texte normal");

  m_doc_designer.appendCRLF();
  m_doc_designer.appendChapterTitle(1, "Nom du chapitre 2");
  m_doc_designer.appendText("Du texte juste en dessous du Chapitre 2");


  m_doc_designer.appendCRLF();
  m_doc_designer.appendChapterTitle(1, "Nom du chapitre 3");
  m_doc_designer.appendText("Une liste à puces");
  QStringList lst;
  lst << "Bonjour" << "Coucou" << "Hello";
  m_doc_designer.appendBulletList(lst);
  m_doc_designer.appendText("Du texte normal juste en dessous de la liste à puces", true);
  m_doc_designer.appendText("La même liste à puce mais en plus gros et en couleur !", true);
  m_doc_designer.setFont(DocFont(m_doc_designer.getDefaultFamilyFont(), 24, true, true, true, Qt::darkBlue));
  m_doc_designer.appendBulletList(lst);


  m_doc_designer.appendCRLF();
  m_doc_designer.appendChapterTitle(1, "Nom du chapitre 4");
  m_doc_designer.appendText("Une belle image redimensionnée et centrée");
  // Ajout d'une image
  m_doc_designer.setAlign(Qt::AlignHCenter);  // Centre l'image sur la page
  m_doc_designer.appendPicture(":/icons/bloc_note.png", 0.6);
  m_doc_designer.appendText("Image 1");

  m_doc_designer.appendCRLF();
  m_doc_designer.appendPicture(":/icons/bloc_note.png", 0.6, false);
  m_doc_designer.appendPicture(":/icons/bloc_note.png", 0.6);
  m_doc_designer.appendText("Deux images côtes à côtes");

  m_doc_designer.appendCRLF();
  m_doc_designer.appendChapterTitle(1, "Nom du chapitre 5");
  m_doc_designer.appendText("Dans ce chapitre, il va y avoir un tableau avec du texte simple et composé : 3 lignes / 4 colonnes");

  DocDesigner doc;  // document intermédiaire
  doc.setFont(DocFont(doc.getDefaultFamilyFont(), 12, true, true, false, QColor(Qt::red), Qt::AlignHCenter));
  doc.appendText("Hello");
  doc.setFont(DocFont("Courier", 14, true, false, true, QColor(Qt::darkGreen)));
  doc.appendText("Du texte en vert");
  doc.setFont(DocFont("Courier", 8, true, false, true, QColor(Qt::red), Qt::AlignHCenter));
  doc.appendText("Du texte en rouge");
  doc.setDefaultFont();
  doc.appendBulletList(lst);
  doc.setAlign(Qt::AlignHCenter);  // Centre l'image
  doc.appendPicture(":/icons/bloc_note.png", 0.5, false);

  DocDesigner doc2;  // document intermédiaire
  doc2.setFont(DocFont(doc2.getDefaultFamilyFont(), 24, true, true, false, QColor(Qt::red), Qt::AlignHCenter));
  doc2.appendText("KO");

  QStringList l1, l2, l3;
  l1 << "L1/C1" << "L1/C2"              << doc2.getHtml()   << "L3/C4";
  l2 << "L2/C1" << doc.getHtml()        << "L2/C3"          << doc.getFormatedText("Courier/Italic", DocFont("Courier", 14, false, true, false));
  l3 << "L3/C1" << "Plein de texte ici" << doc.getHtml()    << "L3/C4";
  QVector<QStringList>lines;
  lines << l1 << l2 << l3;
  m_doc_designer.appendTable(lines);

  m_doc_designer.appendText("Du texte normal juste en dessous du tableau");

  // Export du document dans 3 formats différents :
  m_doc_designer.exportToPdf("Test_exportPdf.pdf");
  m_doc_designer.exportToHtml("Test_exportHtml.html");
  m_doc_designer.exportToOdf("Test_exportODF.odf");

  - Limitations :
    - Pour les images, il peut y avoir un effet de compression sur l'affichage, que l'on ne retrouve pas dans le document généré au format html ou pdf.
    - Dans le document généré au format .odf, les images sont absentes.
    - Il peut y avoir quelques petites différences de rendu en fonction du format exporté (pdf, html, odf).
*/

class DocFont
{
public:
    DocFont(QString family, qreal size, bool bold=false, bool italic=false, bool underline=false, QColor color=Qt::black, Qt::Alignment align=Qt::AlignLeft | Qt::AlignAbsolute)
        : m_family(family), m_size(size), m_bold(bold), m_italic(italic), m_underline(underline), m_color(color), m_align(align)
    { }

    QString         m_family;
    qreal           m_size;
    bool            m_bold;
    bool            m_italic;
    bool            m_underline;
    QColor          m_color;
    Qt::Alignment   m_align;
};

class DocDesigner : public HtmlTextEditor
{
    Q_OBJECT
public:
    explicit DocDesigner(QWidget *parent = 0);

public slots:
    void setDefaultFont();
    void setDefaultFont(const DocFont& font);
    QString getDefaultFamilyFont();
    void setFont(const DocFont& font);
    void setAlign(Qt::Alignment align);

    void appendText(QString text, bool final_crlf=true);
    void appendText(QString text, const DocFont& font, bool final_crlf=true);
    void appendCRLF(unsigned int number=1);
    void appendDocTitle(QString text);
    void appendChapterTitle(int level, QString title, bool final_crlf=false);
    void appendBulletList(QStringList lst, QTextListFormat::Style style=QTextListFormat::ListDisc);
    void appendTable(QVector<QStringList> rows);
    void appendPicture(QString pathfilename, double scale=1.0, bool final_crlf=true);
    void appendPicture(const QImage& image, double scale=1.0, bool final_crlf=true);
    void appendWidget(QWidget* wdgt, double scale=1.0, bool final_crlf=true);
    void appendDoc(DocDesigner *doc);
    void appendHtml(QString html);

    QString getFormatedText(const QString& text, const DocFont& font, bool final_crlf=false);

    void exportToPdf(QString pathfilename);
    void exportToHtml(QString pathfilename);
    void exportToOdf(QString pathfilename);

    QString getHtml();

private :
    int m_current_chapter_level_1;
    int m_current_chapter_level_2;
    int m_current_chapter_level_3;

    DocFont m_default_font;
    DocFont m_current_font;
};

#endif // _DOC_DESIGNER_H_
