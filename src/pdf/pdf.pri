HEADERS      += src/pdf/GraphicsPDFItem.h \
                src/pdf/PDFRenderer.h \
                src/pdf/UBOcrEngine.h \
                src/pdf/UBPdfAnnotations.h \
                src/pdf/UBPdfNavigation.h \
                src/pdf/UBPdfOcr.h \
                src/pdf/UBPdfTextSearch.h \
                src/pdf/XPDFRenderer.h
                
SOURCES      += src/pdf/GraphicsPDFItem.cpp \
                src/pdf/PDFRenderer.cpp \
                src/pdf/UBPdfAnnotations.cpp \
                src/pdf/UBPdfNavigation.cpp \
                src/pdf/UBPdfOcr.cpp \
                src/pdf/UBPdfTextSearch.cpp \
                src/pdf/XPDFRenderer.cpp

# The reader of text of scanned pages: the one of Windows, else tesseract.
win32 {
    SOURCES += src/pdf/UBOcrEngine_win.cpp
    LIBS += -lwindowsapp
} else {
    SOURCES += src/pdf/UBOcrEngine_tesseract.cpp
}
