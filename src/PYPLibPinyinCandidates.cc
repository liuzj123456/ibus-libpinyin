/* vim:set et ts=4 sts=4:
 *
 * ibus-libpinyin - Intelligent Pinyin engine based on libpinyin for IBus
 *
 * Copyright (c) 2018 Peng Wu <alexepico@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "PYPLibPinyinCandidates.h"

using namespace PY;

gboolean
LibPinyinCandidates::processCandidates (std::vector<EnhancedCandidate> & candidates)
{
    pinyin_instance_t *instance = m_editor->m_instance;

    guint len = 0;
    pinyin_get_n_candidate (instance, &len);

    for (guint i = 0; i < len; i++) {
        lookup_candidate_t * candidate = NULL;
        pinyin_get_candidate (instance, i, &candidate);

        const gchar * phrase_string = NULL;
        pinyin_get_candidate_string (instance, candidate, &phrase_string);

        EnhancedCandidate candidate;
        candidate.m_candidate_type = CANDIDATE_LIBPINYIN;
        candidate.m_candidate_id = i;
        candidate.m_display_string = phrase_string;

        candidates.push_back (candidate);
    }

    return TRUE;
}

SelectCandidateAction
LibPinyinCandidates::selectCandidate (EnhancedCandidate & candidate)
{
    pinyin_instance_t * instance = m_editor->m_instance;
    assert (CANDIDATE_LIBPINYIN == candidate.m_candidate_type);

    guint len = 0;
    pinyin_get_n_candidate (instance, &len);

    if (G_UNLIKELY (candidate.m_candidate_id >= len))
        return SELECT_CANDIDATE_ALREADY_HANDLED;

    guint lookup_cursor = m_editor->getLookupCursor ();

    lookup_candidate_t * candidate = NULL;
    pinyin_get_candidate (instance, i, &candidate);

    lookup_candidate_type_t type;
    pinyin_get_candidate_type (instance, candidate, &type);

    if (NBEST_MATCH_CANDIDATE == type) {
        /* because nbest match candidate
           starts from the beginning of user input. */
        pinyin_choose_candidate (instance, 0, candidate);

        guint8 index = 0;
        pinyin_get_candidate_nbest_index(instance, candidate, &index);

        if (index != 0)
            pinyin_train (instance, index);

        return SELECT_CANDIDATE_COMMIT;
    }

    lookup_cursor = pinyin_choose_candidate
        (instance, lookup_cursor, candidate);

    pinyin_guess_sentence (m_instance);

    if (lookup_cursor == m_editor->m_text.length ()) {
        char *tmp = NULL;
        pinyin_get_sentence (instance, 0, &tmp);
        candidate.m_display_string = tmp;
        pinyin_train (m_instance, 0);
        return SELECT_CANDIDATE_MODIFY_IN_PLACE_AND_COMMIT;
    }

    PinyinKeyPos *pos = NULL;
    pinyin_get_pinyin_key_rest (m_instance, lookup_cursor, &pos);

    guint16 begin = 0;
    pinyin_get_pinyin_key_rest_positions (m_instance, pos, &begin, NULL);
    m_editor->m_cursor = begin;

    return SELECT_CANDIDATE_UPDATE_ALL;
}