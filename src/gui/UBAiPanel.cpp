/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */


#include "UBAiPanel.h"

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDrag>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QImageReader>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpression>
#include <QScreen>
#include <QStandardPaths>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWebEngineDownloadRequest>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineView>

#include <functional>

#include "board/UBBoardController.h"
#include "board/UBBoardSaver.h"
#include "board/UBBoardView.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBSettings.h"
#include "domain/UBGraphicsScene.h"

QPointer<UBAiPanel> UBAiPanel::sPanel;

namespace
{
    // Above this size, the file is not given to the page: it is dragged.
    const qint64 sMaximumAttachedSize = 20 * 1024 * 1024;

    QString workFolder()
    {
        const QString folder = UBSettings::userDataDirectory() + "/ai";
        QDir().mkpath(folder);
        return folder;
    }

    /*
     * A part of the board chosen with the mouse: the page darkens, a frame
     * follows the mouse, and the part inside is given as a picture. Escape
     * or a click without dragging gives up.
     */
    class AreaPicker : public QWidget
    {
    public:
        AreaPicker(QWidget* view, std::function<void(const QImage&)> picked)
            : QWidget(view)
            , mView(view)
            , mPicked(std::move(picked))
        {
            setAttribute(Qt::WA_DeleteOnClose);
            setCursor(Qt::CrossCursor);
            setFocusPolicy(Qt::StrongFocus);
            setGeometry(view->rect());
            show();
            raise();
            setFocus();
        }

    protected:
        void paintEvent(QPaintEvent*) override
        {
            QPainter painter(this);
            QPainterPath shade;
            shade.addRect(rect());

            if (!mArea.isNull())
            {
                QPainterPath hole;
                hole.addRect(mArea.normalized());
                shade = shade.subtracted(hole);
            }

            painter.fillPath(shade, QColor(0, 0, 0, 90));
            painter.setPen(QPen(QColor("#2f80ed"), 2, Qt::DashLine));
            painter.drawRect(mArea.normalized());

            // What to do, at the top.
            const QString help = UBAiPanel::tr("Drag over the part of the page to give to the AI. Escape: cancel.");
            QFont font = painter.font();
            font.setPointSize(13);
            font.setBold(true);
            painter.setFont(font);
            const QRect box = painter.fontMetrics().boundingRect(help).adjusted(-14, -8, 14, 8);
            const QRect place(QPoint((width() - box.width()) / 2, 20), box.size());
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(35, 38, 43, 230));
            painter.drawRoundedRect(place, 6, 6);
            painter.setPen(Qt::white);
            painter.drawText(place, Qt::AlignCenter, help);
        }

        void mousePressEvent(QMouseEvent* event) override
        {
            mArea = QRect(event->position().toPoint(), QSize());
            update();
        }

        void mouseMoveEvent(QMouseEvent* event) override
        {
            if (event->buttons() & Qt::LeftButton)
            {
                mArea.setBottomRight(event->position().toPoint());
                update();
            }
        }

        void mouseReleaseEvent(QMouseEvent*) override
        {
            const QRect area = mArea.normalized();

            if (area.width() < 8 || area.height() < 8)
            {
                close();
                return;
            }

            // Without the frame and the shade.
            hide();
            const QImage image = mView->grab(area).toImage();
            const auto picked = mPicked;
            close();
            picked(image);
        }

        void keyPressEvent(QKeyEvent* event) override
        {
            if (event->key() == Qt::Key_Escape)
            {
                close();
            }
        }

    private:
        QWidget* mView;
        std::function<void(const QImage&)> mPicked;
        QRect mArea;
    };

    QString programLanguageName()
    {
        const QString code = UBSettings::settings()->appPreferredLanguage->get().toString();
        const QLocale locale = code.isEmpty() ? QLocale::system() : QLocale(code);

        // The short names of the list; Qt adds the country to some.
        static const QMap<QString, QString> names = {
            { "es", "Español" }, { "en", "English" }, { "de", "Deutsch" }, { "fr", "Français" }, { "it", "Italiano" },
            { "pt", "Português" }, { "zh", "中文" }, { "ja", "日本語" }, { "ko", "한국어" }, { "ru", "Русский" },
            { "ar", "العربية" }, { "nl", "Nederlands" }
        };
        QString name = names.value(locale.name().section('_', 0, 0), locale.nativeLanguageName());

        if (!name.isEmpty())
        {
            name[0] = name.at(0).toUpper();
        }

        return name;
    }

    /** @brief Asks the languages of a translation; false when cancelled. */
    bool askLanguages(QWidget* parent, QString* from, QString* to)
    {
        QDialog dialog(parent);
        dialog.setWindowTitle(UBAiPanel::tr("Translation"));
        dialog.setMinimumWidth(380);

        const QStringList languages = { "Español", "English", "Deutsch", "Français", "Italiano", "Português",
                                        "中文", "日本語", "한국어", "Русский", "العربية", "Nederlands" };

        auto source = new QComboBox(&dialog);
        source->setEditable(true);
        source->addItem(UBAiPanel::tr("Detect it"));
        source->addItems(languages);

        auto target = new QComboBox(&dialog);
        target->setEditable(true);
        target->addItems(languages);
        target->setCurrentText(programLanguageName());

        auto form = new QFormLayout(&dialog);
        form->addRow(new QLabel(UBAiPanel::tr("Write or choose the languages:"), &dialog));
        form->addRow(UBAiPanel::tr("From:"), source);
        form->addRow(UBAiPanel::tr("To:"), target);

        auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        form->addRow(buttons);
        QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() != QDialog::Accepted || target->currentText().trimmed().isEmpty())
        {
            return false;
        }

        *from = source->currentIndex() == 0 && source->currentText() == source->itemText(0)
                    ? QString() : source->currentText().trimmed();
        *to = target->currentText().trimmed();
        return true;
    }

    // The JavaScript that writes the instruction in the box of the AI.
    const char* const sWriteScript = R"JS(
(function (text) {
    const selectors = ['#prompt-textarea', 'rich-textarea div[contenteditable="true"]',
                       'div[contenteditable="true"]', 'textarea'];
    let box = null;
    for (const selector of selectors) {
        const found = [...document.querySelectorAll(selector)].filter(e => e.offsetParent !== null);
        if (found.length) { box = found[found.length - 1]; break; }
    }
    if (!box) return false;
    box.focus();
    if (box.tagName === 'TEXTAREA') {
        const setter = Object.getOwnPropertyDescriptor(HTMLTextAreaElement.prototype, 'value').set;
        setter.call(box, text);
        box.dispatchEvent(new Event('input', { bubbles: true }));
    } else {
        document.execCommand('selectAll', false, null);
        document.execCommand('insertText', false, text);
    }
    return true;
})(%1)
)JS";

    // The JavaScript that gives the file to the page, as if chosen or pasted.
    const char* const sAttachScript = R"JS(
(function (data, name, type) {
    const bytes = atob(data);
    const array = new Uint8Array(bytes.length);
    for (let i = 0; i < bytes.length; i++) array[i] = bytes.charCodeAt(i);
    const transfer = new DataTransfer();
    transfer.items.add(new File([array], name, { type: type }));
    const inputs = [...document.querySelectorAll('input[type=file]')];
    if (inputs.length) {
        const fits = inputs.find(i => !i.accept || i.accept.includes('*') || i.accept.includes(type)
            || i.accept.split(',').some(a => name.toLowerCase().endsWith(a.trim().toLowerCase())));
        const input = fits || inputs[0];
        input.files = transfer.files;
        input.dispatchEvent(new Event('change', { bubbles: true }));
        return 'input';
    }
    const box = document.querySelector('#prompt-textarea, div[contenteditable="true"], textarea');
    if (box) {
        box.dispatchEvent(new ClipboardEvent('paste', { bubbles: true, cancelable: true, clipboardData: transfer }));
        return 'paste';
    }
    return '';
})(%1, %2, %3)
)JS";

    // How many times the page says that a limit is reached.
    const char* const sLimitScript = R"JS(
(function () {
    const text = document.body ? document.body.innerText : '';
    const pattern = /(you('ve| have) (hit|reached)|reached (the|your) (free |daily |usage |message |image )?(limit|cap|quota)|limit reached|out of free|has alcanzado (el|tu) l[ií]mite|alcanzaste (el|tu) l[ií]mite|llegaste al l[ií]mite|l[ií]mite alcanzado|avez atteint (la|votre) limite|limite atteinte|limit erreicht|grenze erreicht)/gi;
    return (text.match(pattern) || []).length;
})()
)JS";

    QString jsString(const QString& text)
    {
        QString quoted = text;
        quoted.replace("\\", "\\\\").replace("'", "\\'").replace("\n", "\\n").replace("\r", "");
        return "'" + quoted + "'";
    }

    QString mimeTypeOf(const QString& file)
    {
        const QString suffix = QFileInfo(file).suffix().toLower();
        static const QMap<QString, QString> types = {
            { "pdf", "application/pdf" }, { "png", "image/png" }, { "jpg", "image/jpeg" }, { "jpeg", "image/jpeg" },
            { "mp3", "audio/mpeg" }, { "wav", "audio/wav" }, { "m4a", "audio/mp4" }, { "ogg", "audio/ogg" },
            { "opus", "audio/ogg" }, { "flac", "audio/flac" }, { "aac", "audio/aac" }, { "webm", "audio/webm" },
            { "wma", "audio/x-ms-wma" }, { "mp4", "audio/mp4" }
        };
        return types.value(suffix, "application/octet-stream");
    }
}

UBAiPanel::UBAiPanel(QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName("aiPanel");
    setStyleSheet("QWidget#aiPanel { background: #23262b; border: 1px solid #5a606a; }"
                  "QLabel { color: white; }"
                  "QLabel#aiFile { background: #3a3f47; border: 1px dashed #8a93a3; border-radius: 4px; padding: 3px 6px; }"
                  "QLabel#aiHint { color: #b8bec8; font-size: 11px; }"
                  "QPushButton { color: white; background: #3a3f47; border: 1px solid #5a606a; border-radius: 4px; padding: 3px 8px; }"
                  "QPushButton:hover { background: #4a505a; }"
                  "QToolButton#aiClose { color: white; background: transparent; border: none; font-size: 18px; padding: 0 6px; }"
                  "QToolButton#aiClose:hover { background: #c42b1c; }");

    // The AI chosen, and the cross that closes the panel.
    mServices = new QComboBox(this);
    mServices->setToolTip(tr("The AI shown; the addresses are in the preferences"));

    for (const Service& service : services())
    {
        mServices->addItem(service.name, service.url);
    }

    auto close = new QToolButton(this);
    close->setObjectName("aiClose");
    close->setText(QStringLiteral("✕"));
    close->setToolTip(tr("Close the AI"));
    connect(close, &QToolButton::clicked, this, &QWidget::close);

    auto top = new QHBoxLayout();
    top->setContentsMargins(8, 4, 2, 0);
    top->addWidget(new QLabel(tr("AI:"), this));
    top->addWidget(mServices, 1);
    top->addWidget(close);

    // The file given to the AI, to drag when the page did not take it.
    mFile = new QLabel(this);
    mFile->setObjectName("aiFile");
    mFile->setCursor(Qt::OpenHandCursor);
    mFile->installEventFilter(this);

    mCopy = new QPushButton(tr("Copy instruction"), this);
    QPushButton* copy = mCopy;
    copy->setToolTip(tr("Copies the instruction again, to paste it with Ctrl+V in the box of the AI"));
    connect(copy, &QPushButton::clicked, this, [this]() {
        QApplication::clipboard()->setText(mInstruction);
    });

    auto paste = new QPushButton(tr("Paste on the page"), this);
    paste->setToolTip(tr("Puts on the page what was copied in the AI: a text, or a picture with its right click, Copy image"));
    connect(paste, &QPushButton::clicked, this, []() {
        UBApplication::applicationController->showBoard();
        UBApplication::boardController->paste();
    });

    auto tools = new QHBoxLayout();
    tools->setContentsMargins(8, 4, 8, 0);
    tools->addWidget(mFile, 1);
    tools->addWidget(copy);
    tools->addWidget(paste);

    mInstructionLabel = new QLabel(this);
    mInstructionLabel->setObjectName("aiHint");
    mInstructionLabel->setWordWrap(true);
    mInstructionLabel->setContentsMargins(8, 2, 8, 4);

    mPage = new QWebEngineView(this);
    mPage->setPage(new QWebEnginePage(profile(), mPage));
    connect(mPage, &QWebEngineView::loadFinished, this, [this]() {
        if (!mInstruction.isEmpty() && mLimitMarks < 0)
        {
            // Some pages make their box a moment after loading.
            QTimer::singleShot(1500, this, [this]() { fillPage(0); });
        }
    });

    // The left edge, dragged, makes the panel wider or narrower.
    mEdge = new QWidget(this);
    mEdge->setFixedWidth(6);
    mEdge->setCursor(Qt::SizeHorCursor);
    mEdge->setToolTip(tr("Drag to make the panel wider"));
    mEdge->installEventFilter(this);

    auto column = new QVBoxLayout();
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(0);
    column->addLayout(top);
    column->addLayout(tools);
    column->addWidget(mInstructionLabel);
    column->addWidget(mPage, 1);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(0);
    layout->addWidget(mEdge);
    layout->addLayout(column, 1);

    mLimitTimer = new QTimer(this);
    mLimitTimer->setInterval(4000);
    connect(mLimitTimer, &QTimer::timeout, this, &UBAiPanel::checkLimit);

    const int current = qBound(0, UBSettings::settings()->aiCurrentService->get().toInt(), qMax(0, mServices->count() - 1));
    mServices->setCurrentIndex(current);
    connect(mServices, qOverload<int>(&QComboBox::activated), this, &UBAiPanel::loadService);

    updateFileLabel();
    loadService(current);
}

QStringList UBAiPanel::defaultServices()
{
    return { "ChatGPT|https://chatgpt.com/", "Claude|https://claude.ai/new", "Gemini|https://gemini.google.com/app",
             "Copilot|https://copilot.microsoft.com/" };
}

QList<UBAiPanel::Service> UBAiPanel::services()
{
    QList<Service> result;
    const QStringList saved = UBSettings::settings()->aiServices->get().toStringList();

    for (const QString& entry : saved)
    {
        const int bar = entry.indexOf('|');
        const QString name = entry.left(bar).trimmed();
        QUrl url = QUrl::fromUserInput(entry.mid(bar + 1).trimmed());

        if (bar > 0 && url.isValid() && !name.isEmpty())
        {
            result << Service{ name, url };
        }
    }

    if (result.isEmpty())
    {
        for (const QString& entry : defaultServices())
        {
            result << Service{ entry.section('|', 0, 0), QUrl(entry.section('|', 1)) };
        }
    }

    return result;
}

QWebEngineProfile* UBAiPanel::profile()
{
    static QWebEngineProfile* aiProfile = nullptr;

    if (!aiProfile)
    {
        // Its own profile keeps the sessions of the AI between the openings.
        aiProfile = new QWebEngineProfile(QStringLiteral("LibertyBoardAi"), qApp);
        aiProfile->setPersistentStoragePath(UBSettings::userDataDirectory() + "/webprofiles/ai");
        aiProfile->setCachePath(UBSettings::userDataDirectory() + "/webprofiles/ai/cache");
        aiProfile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);

        // Some pages refuse to sign in from a browser that names itself QtWebEngine.
        QString agent = aiProfile->httpUserAgent();
        agent.remove(QRegularExpression("QtWebEngine/[^ ]+ ?"));
        aiProfile->setHttpUserAgent(agent);

        aiProfile->settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
        aiProfile->settings()->setAttribute(QWebEngineSettings::JavascriptCanPaste, true);
        connect(aiProfile, &QWebEngineProfile::downloadRequested, aiProfile, &UBAiPanel::downloaded);
    }

    return aiProfile;
}

void UBAiPanel::downloaded(QWebEngineDownloadRequest* download)
{
    // What the AI gives to download goes to Downloads; a picture also to the page.
    const QString folder = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    download->setDownloadDirectory(folder);
    download->accept();

    connect(download, &QWebEngineDownloadRequest::isFinishedChanged, download, [download]() {
        if (download->state() != QWebEngineDownloadRequest::DownloadCompleted)
        {
            return;
        }

        const QString file = QDir(download->downloadDirectory()).filePath(download->downloadFileName());
        QFile picture(file);

        if (QImageReader(file).canRead() && picture.open(QIODevice::ReadOnly) && UBApplication::boardController->activeScene())
        {
            UBApplication::applicationController->showBoard();
            UBBoardView* view = UBApplication::boardController->controlView();
            const QPointF center = view->mapToScene(view->viewport()->rect().center());
            UBApplication::boardController->activeScene()->addImage(picture.readAll(), nullptr, center, 1.);
            UBApplication::showMessage(tr("The picture of the AI is on the page; it is also in %1").arg(QDir::toNativeSeparators(file)));
        }
        else
        {
            UBApplication::showMessage(tr("Downloaded: %1").arg(QDir::toNativeSeparators(file)));
        }
    });
}

UBAiPanel* UBAiPanel::open(QWidget* parent)
{
    if (!sPanel)
    {
        sPanel = new UBAiPanel(parent ? parent->window() : nullptr);
        sPanel->placeBeside(parent);
    }

    sPanel->show();
    sPanel->raise();
    sPanel->activateWindow();
    return sPanel;
}

void UBAiPanel::toggle(QWidget* parent)
{
    if (sPanel)
    {
        sPanel->close();
        return;
    }

    open(parent);
}

void UBAiPanel::showActions(QWidget* parent)
{
    QMenu menu(parent);
    const QList<QPair<Action, QString>> actions = {
        { Action::Summary, tr("Summary") },
        { Action::Translation, tr("Translation") },
        { Action::CreateImage, tr("Create an image") },
        { Action::RemoveBackground, tr("Remove the background") },
        { Action::GenerativeErase, tr("Generative erasing") },
        { Action::Transcription, tr("Transcribe an audio") },
    };

    for (const auto& action : actions)
    {
        QAction* item = menu.addAction(action.second);
        const Action chosen = action.first;
        connect(item, &QAction::triggered, parent, [chosen, parent]() { startAction(chosen, parent); });
    }

    menu.addSeparator();
    QAction* panel = menu.addAction(sPanel ? tr("Close the AI") : tr("Only open the AI"));
    connect(panel, &QAction::triggered, parent, [parent]() { toggle(parent); });

    menu.exec(QCursor::pos());
}

void UBAiPanel::startAction(Action action, QWidget* parent)
{
    // What the AI needs to know, asked before choosing the part.
    QString detail;
    QString from;
    QString to;

    if (action == Action::Translation && !askLanguages(parent, &from, &to))
    {
        return;
    }

    if (action == Action::CreateImage || action == Action::GenerativeErase)
    {
        bool ok = false;
        detail = action == Action::CreateImage
                     ? QInputDialog::getMultiLineText(parent, tr("Create an image"), tr("Describe the image to create:"), QString(), &ok)
                     : QInputDialog::getMultiLineText(parent, tr("Generative erasing"), tr("What should the AI erase from the picture?"), QString(), &ok);

        if (!ok || detail.trimmed().isEmpty())
        {
            return;
        }

        detail = detail.trimmed();
    }

    QString instruction;

    switch (action)
    {
    case Action::Summary:
        instruction = tr("Summarize the attached document in clear words: first the main idea in two or three sentences, "
                         "then the important points as a list.");
        break;
    case Action::Translation:
        instruction = from.isEmpty()
                          ? tr("Translate the text of the attached file into %1. Keep its order and its titles, and give only the translation.").arg(to)
                          : tr("Translate the text of the attached file from %1 into %2. Keep its order and its titles, and give only the translation.").arg(from, to);
        break;
    case Action::CreateImage:
        instruction = tr("Create an image: %1").arg(detail);
        break;
    case Action::RemoveBackground:
        instruction = tr("Remove the background of the attached image and give me the result as a PNG image "
                         "with a transparent background, of the same size.");
        break;
    case Action::GenerativeErase:
        instruction = tr("In the attached image, erase this: %1. Fill its place so that it looks natural, as if it had never been there, "
                         "and give me the result as an image of the same size.").arg(detail);
        break;
    case Action::Transcription:
        instruction = tr("Transcribe the attached audio word by word. When several people speak, mark who speaks. "
                         "Give only the text.");
        break;
    }

    // An image is made from its description alone.
    if (action == Action::CreateImage)
    {
        open(parent)->setWork(instruction, QString());
        return;
    }

    if (action == Action::Transcription)
    {
        const QString audio = QFileDialog::getOpenFileName(parent, tr("Audio to transcribe"), QString(),
                                                           tr("Audio") + " (*.mp3 *.wav *.m4a *.ogg *.opus *.flac *.aac *.wma *.webm *.mp4)");

        if (!audio.isEmpty())
        {
            open(parent)->setWork(instruction, audio);
        }

        return;
    }

    // A part of the page, or all of it.
    const bool picture = action == Action::RemoveBackground || action == Action::GenerativeErase;
    QMessageBox question(QMessageBox::Question, tr("AI"), tr("What should be given to the AI?"), QMessageBox::Cancel, parent);
    QPushButton* part = question.addButton(tr("A part of the page"), QMessageBox::AcceptRole);
    QPushButton* whole = question.addButton(picture ? tr("The whole page") : tr("The whole PDF or board"), QMessageBox::AcceptRole);
    question.exec();

    if (question.clickedButton() == whole)
    {
        UBApplication::applicationController->showBoard();
        const QString file = workFolder() + (picture ? "/LibertyBoard-page.png" : "/LibertyBoard-document.pdf");
        UBBoardSaver saver;

        if (!saver.saveTo(file))
        {
            QMessageBox::warning(parent, tr("AI"), tr("The file for the AI could not be made."));
            return;
        }

        open(parent)->setWork(instruction, file);
    }
    else if (question.clickedButton() == part)
    {
        UBApplication::applicationController->showBoard();
        UBBoardView* view = UBApplication::boardController->controlView();
        QPointer<QWidget> owner = parent;

        new AreaPicker(view, [instruction, owner](const QImage& image) {
            const QString file = workFolder() + "/LibertyBoard-selection.png";

            if (!image.save(file, "PNG"))
            {
                QMessageBox::warning(owner, tr("AI"), tr("The file for the AI could not be made."));
                return;
            }

            open(owner)->setWork(instruction, file);
        });
    }
}

void UBAiPanel::setWork(const QString& instruction, const QString& file)
{
    mInstruction = instruction;
    mFilePath = file;
    mLimitShown = false;
    QApplication::clipboard()->setText(instruction);
    updateFileLabel();

    // The instruction goes in a new conversation.
    loadService(mServices->currentIndex());
}

void UBAiPanel::updateFileLabel()
{
    mCopy->setVisible(!mInstruction.isEmpty());

    if (mInstruction.isEmpty())
    {
        mFile->hide();
        mInstructionLabel->setText(tr("Drag a text or a picture of the AI to the page, or copy it and use Paste on the page."));
        return;
    }

    mFile->setVisible(!mFilePath.isEmpty());
    mFile->setText(QStringLiteral("📎 ") + QFileInfo(mFilePath).fileName());
    mFile->setToolTip(tr("If the AI did not take the file, drag it from here to the AI"));
    mInstructionLabel->setText(tr("The instruction is written in the AI and also copied: Ctrl+V pastes it. "
                                  "Drag the answer to the page, or use Paste on the page."));
    mInstructionLabel->setToolTip(mInstruction);
}

void UBAiPanel::loadService(int index)
{
    if (index < 0)
    {
        return;
    }

    UBSettings::settings()->aiCurrentService->set(index);
    mLimitMarks = -1;
    mLimitTimer->stop();
    mPage->load(mServices->itemData(index).toUrl());
}

void UBAiPanel::fillPage(int attempt)
{
    const QString script = QString::fromUtf8(sWriteScript).arg(jsString(mInstruction));

    mPage->page()->runJavaScript(script, [this, attempt](const QVariant& written) {
        if (!written.toBool())
        {
            // Not ready yet, or a page asking to sign in: the clipboard has it.
            if (attempt < 12)
            {
                QTimer::singleShot(1500, this, [this, attempt]() { fillPage(attempt + 1); });
            }

            return;
        }

        // The file, as if chosen in the page; else it is dragged from the panel.
        QFile file(mFilePath);

        if (!mFilePath.isEmpty() && file.size() <= sMaximumAttachedSize && file.open(QIODevice::ReadOnly))
        {
            const QString attach = QString::fromUtf8(sAttachScript)
                                       .arg("'" + QString::fromLatin1(file.readAll().toBase64()) + "'",
                                            jsString(QFileInfo(mFilePath).fileName()), jsString(mimeTypeOf(mFilePath)));
            mPage->page()->runJavaScript(attach);
        }

        // From now on, a new message of limit means that the free uses are over.
        mPage->page()->runJavaScript(QString::fromUtf8(sLimitScript), [this](const QVariant& marks) {
            mLimitMarks = marks.toInt();
            mLimitTimer->start();
        });
    });
}

void UBAiPanel::checkLimit()
{
    if (mLimitMarks < 0 || mLimitShown)
    {
        return;
    }

    mPage->page()->runJavaScript(QString::fromUtf8(sLimitScript), [this](const QVariant& marks) {
        if (marks.toInt() > mLimitMarks && !mLimitShown)
        {
            mLimitShown = true;
            offerOtherService();
        }
    });
}

void UBAiPanel::offerOtherService()
{
    QMessageBox question(QMessageBox::Warning, tr("AI"),
                         tr("It seems that the free uses of %1 are over for now. Do the same with another AI?")
                             .arg(mServices->currentText()),
                         QMessageBox::NoButton, parentWidget());
    QMap<QAbstractButton*, int> choices;

    for (int i = 0; i < mServices->count(); ++i)
    {
        if (i != mServices->currentIndex())
        {
            choices[question.addButton(mServices->itemText(i), QMessageBox::AcceptRole)] = i;
        }
    }

    question.addButton(tr("Stay here"), QMessageBox::RejectRole);
    question.exec();

    if (choices.contains(question.clickedButton()))
    {
        mServices->setCurrentIndex(choices.value(question.clickedButton()));
        mLimitShown = false;
        loadService(mServices->currentIndex());
    }
}

bool UBAiPanel::isAiDrag(const QMimeData* mimeData)
{
    if (!mimeData->hasImage())
    {
        return false;
    }

    QList<QUrl> urls = mimeData->urls();

    if (mimeData->hasHtml())
    {
        const QString address = UBApplication::urlFromHtml(mimeData->html());

        if (!address.isEmpty())
        {
            urls << QUrl(address);
        }
    }

    QStringList hosts;

    for (const Service& service : services())
    {
        hosts << service.url.host().toLower().remove(QRegularExpression("^www\\."));
    }

    for (const QUrl& url : urls)
    {
        const QString host = url.host().toLower();

        if (url.scheme() == "blob" || url.scheme() == "data")
        {
            return true;
        }

        for (const QString& aiHost : hosts)
        {
            // The pictures of an AI often come from another address of the same company.
            const QString company = aiHost.section('.', -2);

            if (host == aiHost || host.endsWith("." + company) || host == company
                    || host.contains("oaiusercontent") || host.contains("googleusercontent"))
            {
                return true;
            }
        }
    }

    return false;
}

void UBAiPanel::placeBeside(QWidget* beside)
{
    // A column at the right of the window, or of the screen.
    QRect area = beside ? beside->window()->geometry() : QRect();
    QScreen* screen = beside ? beside->screen() : nullptr;

    if (screen && (area.isEmpty() || beside->window()->isFullScreen() || area.width() >= screen->availableGeometry().width()))
    {
        area = screen->availableGeometry();
    }

    if (area.isEmpty())
    {
        resize(520, 760);
        return;
    }

    const int width = qBound(420, area.width() / 4, 700);
    const int top = area.top() + qMin(110, area.height() / 8);
    // Left of the tool palette, which is at the right.
    setGeometry(area.right() - width - 110, top, width, area.bottom() - top - 20);
}

bool UBAiPanel::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == mEdge)
    {
        // The left edge follows the mouse; the right edge stays.
        auto mouse = static_cast<QMouseEvent*>(event);

        if (event->type() == QEvent::MouseButtonPress)
        {
            mResizeStartX = mouse->globalPosition().toPoint().x();
            mResizeStartGeometry = geometry();
            return true;
        }

        if (event->type() == QEvent::MouseMove && (mouse->buttons() & Qt::LeftButton))
        {
            const int moved = mouse->globalPosition().toPoint().x() - mResizeStartX;
            const int width = qMax(320, mResizeStartGeometry.width() - moved);
            setGeometry(mResizeStartGeometry.right() + 1 - width, mResizeStartGeometry.top(), width, mResizeStartGeometry.height());
            return true;
        }
    }
    else if (watched == mFile)
    {
        // The file, dragged to the page of the AI or elsewhere.
        auto mouse = static_cast<QMouseEvent*>(event);

        if (event->type() == QEvent::MouseButtonPress)
        {
            mDragStart = mouse->position().toPoint();
            return true;
        }

        if (event->type() == QEvent::MouseMove && (mouse->buttons() & Qt::LeftButton)
                && (mouse->position().toPoint() - mDragStart).manhattanLength() > QApplication::startDragDistance()
                && !mFilePath.isEmpty())
        {
            auto data = new QMimeData();
            data->setUrls({ QUrl::fromLocalFile(mFilePath) });
            auto drag = new QDrag(mFile);
            drag->setMimeData(data);
            drag->exec(Qt::CopyAction);
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void UBAiPanel::closeEvent(QCloseEvent* event)
{
    if (sPanel == this)
    {
        sPanel.clear();
    }

    QWidget::closeEvent(event);
}
