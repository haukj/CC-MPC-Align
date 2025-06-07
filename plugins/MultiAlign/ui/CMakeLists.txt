qt_wrap_ui(UI_HEADERS
    MultiAlignDlg.ui
)

target_sources(${PROJECT_NAME} PRIVATE
    ${UI_HEADERS}
)
