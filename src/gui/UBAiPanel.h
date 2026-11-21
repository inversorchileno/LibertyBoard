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


#pragma once

#include <QPointer>
#include <QStringList>
#include <QUrl>
#include <QWidget>

class QComboBox;
class QLabel;
class QMimeData;
class QPushButton;
class QTimer;
class QWebEngineDownloadRequest;
class QWebEngineProfile;
class QWebEngineView;

/**
 * @brief The panel of the AI, at the right of the board: the page of the AI
 * chosen (ChatGPT, Claude, Gemini, Copilot...), with its session kept.
 *
 * The AI button asks first what to do (summary, translation, image,
 * background, erasing, transcription), then on what (a part chosen on the
 * page, or the whole PDF or canvas), exports it, and opens the panel with
 * the instruction written for the AI and the file attached, when the page
 * of the AI lets it; else the instruction is in the clipboard and the file
 * is dragged from the panel. What the AI gives back is dragged to the page,
 * pasted, or put on the page when it is downloaded.
 */
class UBAiPanel : public QWidget
{
    Q_OBJECT

public:
    enum class Action
    {
        Summary,
        Translation,
        CreateImage,
        RemoveBackground,
        GenerativeErase,
        Transcription
    };

    struct Service
    {
        QString name;
        QUrl url;
    };

    /** @brief The AI of the preferences, as "name|address". */
    static QStringList defaultServices();
    static QList<Service> services();

    /** @brief Asks what is needed for the action, exports, and opens the panel. */
    static void startAction(Action action, QWidget* parent);

    /** @brief Opens the panel or closes it, without action. */
    static void toggle(QWidget* parent);

    /** @brief Puts a picture over the desktop on the desktop, else on the page, at the center or there. */
    static void putPicture(const QByteArray& data, const QPointF* position = nullptr);
    static QByteArray pngOf(const QImage& image);

    /** @brief Puts a text over the desktop on the desktop, else on the page, at the center. */
    static void putText(const QString& text);

    /**
     * @brief Where a side window goes, as the panel of the AI and the window
     * of the GIFs: against the edge opposite the tool palette, without
     * covering the bar of options, the zoom or a palette lying across. On
     * the desktop, in the coordinates of the layer over the desktop.
     */
    static QRect sidePlace(bool desktop, QWidget* layer);

    /** @brief Where the panel is in the layer over the desktop, or nothing. */
    static QRect desktopArea();

    /**
     * @brief A drag from the page of an AI: its picture goes on the page, or
     * on the desktop, read in the page of the AI when its address needs the
     * session. False when the drag is not from an AI.
     */
    static bool takeDrop(const QMimeData* mimeData, const QPointF& position);

protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    UBAiPanel(QWidget* parent, bool desktop);

    static UBAiPanel* open(QWidget* parent);
    static QWebEngineProfile* profile();
    static void downloaded(QWebEngineDownloadRequest* download);

    /** @brief Gives the next work: its instruction and its file, maybe none. */
    void setWork(const QString& instruction, const QString& file);
    void loadService(int index);
    void fillPage(int attempt);
    void sendMessage(int attempt);
    void fetchPicture(const QUrl& address, const QPointF& position, int attempt);
    void checkLimit();
    void offerOtherService();
    void placeBeside(QWidget* beside);
    void updateFileLabel();

private:
    QWebEngineView* mPage{nullptr};
    QComboBox* mServices{nullptr};
    QLabel* mFile{nullptr};
    QPushButton* mCopy{nullptr};
    QLabel* mInstructionLabel{nullptr};
    QWidget* mEdge{nullptr};
    QTimer* mLimitTimer{nullptr};
    bool mDesktop{false};
    QString mInstruction;
    QString mFilePath;
    int mLimitMarks{-1};
    bool mLimitShown{false};
    QPoint mDragStart;
    int mResizeStartX{0};
    QRect mResizeStartGeometry;
    static QPointer<UBAiPanel> sPanel;
};
