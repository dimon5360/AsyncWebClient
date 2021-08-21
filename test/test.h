/******************************************************
 *  @file   test.h
 *  @brief  Unit tests declarations
 *
 *  @author Kalmykov Dmitry
 *  @date   11.08.2021
 */

#pragma once

#define UNIT_TEST true

#if UNIT_TEST
/* Application GUI tests ----------------- */
#define GUI_APP_INIT                    0
#define GUI_APP_WITH_CLIENT_INIT        0
#define SEPARATED_GUI_WITH_LOGIC        1


extern void init_unit_tests();

#endif /* UNIT_TEST */