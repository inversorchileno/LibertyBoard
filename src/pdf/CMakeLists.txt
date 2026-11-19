target_sources(${PROJECT_NAME} PRIVATE
    GraphicsPDFItem.cpp
    GraphicsPDFItem.h
    PDFRenderer.cpp
    PDFRenderer.h
    UBOcrEngine.h
    UBPdfAnnotations.cpp
    UBPdfAnnotations.h
    UBPdfNavigation.cpp
    UBPdfNavigation.h
    UBPdfOcr.cpp
    UBPdfOcr.h
    UBPdfTextSearch.cpp
    UBPdfTextSearch.h
    XPDFRenderer.cpp
    XPDFRenderer.h
)

# The reader of text of scanned pages: the one of Windows, else tesseract.
if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE UBOcrEngine_win.cpp)
    target_link_libraries(${PROJECT_NAME} windowsapp)
else()
    target_sources(${PROJECT_NAME} PRIVATE UBOcrEngine_tesseract.cpp)
endif()
