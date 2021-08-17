
#pragma once

#define UNIT_TEST true

#if UNIT_TEST
/* Application GUI tests ----------------- */
#define GUI_APP_INIT                    0
#define GUI_APP_WITH_CLIENT_INIT        1


extern void init_unit_tests();

#endif /* UNIT_TEST */