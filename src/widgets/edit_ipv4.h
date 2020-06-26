#include "widgets/edit.h"

#define EDIT_IPV4_SEP '.'
#define EDIT_IPV4_SEP_NR 3
#define EDIT_IPV4_MIN_PART_SIZE 1
#define EDIT_IPV4_MAX_PART_SIZE 3
#define EDIT_IPV4_PART_VALUE_IS_VALID(index, v) ((v) >= 0 && (v) <= 255)
#define EDIT_IPV4_PART_SIZE_IS_VALID(len) \
  ((len) >= EDIT_IPV4_MIN_PART_SIZE && (len) <= EDIT_IPV4_MAX_PART_SIZE)
#define EDIT_IPV4_FIX_PART(index, part_start, part_end, v) \
  if ((v) > 255) {                                         \
    (part_end)--;                                          \
  }

static bool_t edit_ipv4_is_valid(widget_t* widget) {
  uint32_t i = 0;
  const wchar_t* ps = NULL;
  const wchar_t* pe = NULL;
  wstr_t* text = &(widget->text);

  if (tk_wstr_count_c(text->str, EDIT_IPV4_SEP) != EDIT_IPV4_SEP_NR) {
    return FALSE;
  }

  ps = text->str;
  pe = wcs_chr(ps, EDIT_IPV4_SEP);

  do {
    int v = 0;
    int len = pe - ps;
    if (!EDIT_IPV4_PART_SIZE_IS_VALID(len)) {
      return FALSE;
    }

    v = tk_watoi_n(ps, pe - ps);
    if (!(EDIT_IPV4_PART_VALUE_IS_VALID(i, v))) {
      return FALSE;
    }

    if (i >= EDIT_IPV4_SEP_NR) {
      break;
    }

    i++;
    ps = pe + 1;
    pe = wcs_chr(ps, EDIT_IPV4_SEP);
    if (pe == NULL) {
      pe = ps + wcslen(ps);
    }
  } while (TRUE);

  return TRUE;
}

static ret_t edit_ipv4_inc_value(widget_t* widget) {
  return edit_add_value_with_sep(widget, 1, EDIT_IPV4_SEP);
}

static ret_t edit_ipv4_dec_value(widget_t* widget) {
  return edit_add_value_with_sep(widget, -1, EDIT_IPV4_SEP);
}

static ret_t edit_ipv4_fix_ex(widget_t* widget, bool_t strict) {
  uint32_t i = 0;
  wchar_t str[32];
  wchar_t* p = str;
  wchar_t* pd = str;
  const wchar_t* ps = NULL;
  const wchar_t* pe = NULL;
  wstr_t* text = &(widget->text);

  memset(str, 0x00, sizeof(str));
  if (tk_wstr_count_c(text->str, EDIT_IPV4_SEP) != 3) {
    widget_set_text_utf8(widget, "0.0.0.0");
    edit_set_cursor(widget, 0);

    return RET_OK;
  }

  ps = text->str;
  pe = wcs_chr(ps, EDIT_IPV4_SEP);

  do {
    int v = 0;
    while (ps != pe && (p - pd) < EDIT_IPV4_MAX_PART_SIZE) {
      *p++ = *ps++;
    }

    if (strict) {
      v = tk_watoi_n(pd, p - pd);
      EDIT_IPV4_FIX_PART(i, pd, p, v);
    }

    if (i == EDIT_IPV4_SEP_NR) {
      *p = 0;
      break;
    } else {
      *p++ = '.';
    }

    i++;
    pd = p;
    ps = pe + 1;
    pe = wcs_chr(ps, EDIT_IPV4_SEP);
    if (pe == NULL) {
      pe = ps + wcslen(ps);
    }
  } while (TRUE);

  widget_set_text(widget, str);

  return RET_OK;
}

static ret_t edit_ipv4_fix(widget_t* widget) {
  return edit_ipv4_fix_ex(widget, TRUE);
}

static bool_t edit_ipv4_is_valid_char(widget_t* widget, wchar_t c) {
  edit_ipv4_fix_ex(widget, FALSE);

  if ((c >= '0' && c <= '9') || c == EDIT_IPV4_SEP) {
    uint32_t cursor = 0;
    text_edit_state_t state;
    edit_t* edit = EDIT(widget);
    wstr_t* text = &(widget->text);
    text_edit_get_state(edit->model, &state);

    cursor = state.cursor <= text->size ? state.cursor : text->size;

    if (c == EDIT_IPV4_SEP) {
      if (text->str[cursor] == EDIT_IPV4_SEP) {
        /*如果时sep，而且当前字符也时sep，光标往后移动一个位置*/
        text_edit_set_cursor(edit->model, cursor + 1);
      }
      return FALSE;
    }

    if (text->str[cursor] == EDIT_IPV4_SEP || text->str[cursor] == 0) {
      int len = 0;
      const wchar_t* pe = text->str + cursor;
      const wchar_t* ps = pe - 1;

      /*找到当前part的第一个字符*/
      while (ps > text->str && *ps != EDIT_IPV4_SEP) {
        ps--;
      }

      if (*ps == EDIT_IPV4_SEP) {
        ps++;
      }

      len = pe - ps;
      /*part 字符超长*/
      if (len >= EDIT_IPV4_MAX_PART_SIZE) {
        if (text->str[cursor] == 0) {
          return FALSE;
        } else if (text->str[cursor] == EDIT_IPV4_SEP) {
          /*输入到下一个part*/
          text_edit_set_cursor(edit->model, cursor + 1);
        } else {
          /*覆盖下一个字符*/
          text_edit_set_select(edit->model, cursor + 1, cursor + 2);
        }
      } else {
        /*正常输入*/
        text_edit_set_cursor(edit->model, cursor);
      }
    } else {
      /*覆盖下一个字符*/
      text_edit_set_select(edit->model, cursor, cursor + 1);
    }

    return TRUE;
  }

  return FALSE;
}

static ret_t edit_ipv4_pre_input(widget_t* widget, uint32_t key) {
  return edit_pre_input_with_sep(widget, key, '.');
}
