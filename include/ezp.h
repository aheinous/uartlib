#pragma once

#ifdef __cplusplus
extern "C" {
#endif


// -----------------------------------------------------------------------------------------
// ------------------------- X Macro Msg Table Definitions ---------------------------------
// -----------------------------------------------------------------------------------------


// Convienience methods ------------------ -------------------------------------------------

#define START_MSG(msgName)      EZP_RESULT ezp_send_ ## msgName ##  (ezp_master_t *master
#define FIELD(type, fieldName) , type fieldName
#define END_MSG(msgName)        );



    #include EZP_MSG_TABLE


#undef START_MSG
#undef END_MSG
#undef FIELD

// -----------------------------------------------------------------------------------------
// ------------------------- End X Macro Msg Table Definitions -----------------------------
// -----------------------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif