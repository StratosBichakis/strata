# --- Configuration ---
set(REMOTE_USER "root")
set(REMOTE_HOST "bela.local")
set(REMOTE_DEST "/${REMOTE_USER}/Bela/projects/")

# --- Function to add Deploy/Run targets for any executable ---
function(add_bela_deploy_target TARGET_NAME)
    # The deploy target stays the same
    add_custom_target(deploy-${TARGET_NAME}
            COMMAND scp $<TARGET_FILE:${TARGET_NAME}> ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_DEST}
            COMMAND scp ${CMAKE_CURRENT_BINARY_DIR}/*.pl ${CMAKE_CURRENT_BINARY_DIR}/*.pm ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_DEST}
            USES_TERMINAL
            DEPENDS ${TARGET_NAME}
            COMMENT "--- 🚀 Deploying ${TARGET_NAME} ---"
    )

    # We add 'USES_TERMINAL' and a dummy command to satisfy the IDE
    add_custom_target(run-${TARGET_NAME}
            COMMAND scp $<TARGET_FILE:${TARGET_NAME}> ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_DEST}
            COMMAND scp ${CMAKE_CURRENT_BINARY_DIR}/*.pl ${CMAKE_CURRENT_BINARY_DIR}/*.pm ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_DEST}
            COMMAND ssh -t ${REMOTE_USER}@${REMOTE_HOST} "${REMOTE_DEST}${TARGET_NAME}"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            USES_TERMINAL
            DEPENDS ${TARGET_NAME}
            COMMENT "--- ⚡ Running ${TARGET_NAME} ---"
    )
endfunction()
