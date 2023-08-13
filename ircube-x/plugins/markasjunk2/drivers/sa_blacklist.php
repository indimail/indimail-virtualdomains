<?php

/**
 * SpamAssassin Blacklist driver
 *
 * @version 2.0
 * @requires SAUserPrefs plugin
 *
 * @author Philip Weir
 *
 * Copyright (C) 2010-2014 Philip Weir
 *
 * This driver is part of the MarkASJunk2 plugin for Roundcube.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Roundcube. If not, see https://www.gnu.org/licenses/.
 */
class markasjunk2_sa_blacklist
{
    private $sa_user;
    private $sa_table;
    private $sa_username_field;
    private $sa_preference_field;
    private $sa_value_field;

    public function spam($uids, $src_mbox, $dst_mbox)
    {
        $this->_do_list($uids, true);
    }

    public function ham($uids, $src_mbox, $dst_mbox)
    {
        $this->_do_list($uids, false);
    }

    private function _do_list($uids, $spam)
    {
        $rcube = rcube::get_instance();
        $this->sa_user = $rcube->config->get('sauserprefs_userid', "%u");
        $this->sa_table = $rcube->config->get('sauserprefs_sql_table_name');
        $this->sa_username_field = $rcube->config->get('sauserprefs_sql_username_field');
        $this->sa_preference_field = $rcube->config->get('sauserprefs_sql_preference_field');
        $this->sa_value_field = $rcube->config->get('sauserprefs_sql_value_field');

        $identity_arr = $rcube->user->get_identity();
        $identity = $identity_arr['email'];
        $this->sa_user = str_replace('%u', $_SESSION['username'], $this->sa_user);
        $this->sa_user = str_replace('%l', $rcube->user->get_username('local'), $this->sa_user);
        $this->sa_user = str_replace('%d', $rcube->user->get_username('domain'), $this->sa_user);
        $this->sa_user = str_replace('%i', $identity, $this->sa_user);

        if (!$rcube->config->load_from_file($rcube->config->get('markasjunk2_sauserprefs_config'))) {
            rcube::raise_error(array('code' => 527, 'type' => 'php',
                'file' => __FILE__, 'line' => __LINE__,
                'message' => "Failed to load config from " . $rcube->config->get('markasjunk2_sauserprefs_config')
            ), true, false);

            return false;
        }

        $db = rcube_db::factory($rcube->config->get('sauserprefs_db_dsnw'), $rcube->config->get('sauserprefs_db_dsnr'), $rcube->config->get('sauserprefs_db_persistent'));
        $db->set_debug((bool) $rcube->config->get('sql_debug'));
        $db->db_connect('w');

        // check DB connections and exit on failure
        if ($err_str = $db->is_error()) {
            rcube::raise_error(array(
                'code' => 603,
                'type' => 'db',
                'message' => $err_str
            ), false, true);
        }

        foreach ($uids as $uid) {
            $message = new rcube_message($uid);
            $email = $message->sender['mailto'];

            if ($spam) {
                // delete any whitelisting for this address
                $db->query(
                    "DELETE FROM `{$this->sa_table}` WHERE `{$this->sa_username_field}` = ? AND `{$this->sa_preference_field}` = ? AND `{$this->sa_value_field}` = ?;",
                    $this->sa_user,
                    'whitelist_from',
                    $email);

                // check address is not already blacklisted
                $sql_result = $db->query(
                                "SELECT `value` FROM `{$this->sa_table}` WHERE `{$this->sa_username_field}` = ? AND `{$this->sa_preference_field}` = ? AND `{$this->sa_value_field}` = ?;",
                                $this->sa_user,
                                'blacklist_from',
                                $email);

                if (!$db->fetch_array($sql_result)) {
                    $db->query(
                        "INSERT INTO `{$this->sa_table}` (`{$this->sa_username_field}`, `{$this->sa_preference_field}`, `{$this->sa_value_field}`) VALUES (?, ?, ?);",
                        $this->sa_user,
                        'blacklist_from',
                        $email);

                    if ($rcube->config->get('markasjunk2_debug')) {
                        rcube::write_log('markasjunk2', $this->sa_user . ' blacklist ' . $email);
                    }
                }
            }
            else {
                // delete any blacklisting for this address
                $db->query(
                    "DELETE FROM `{$this->sa_table}` WHERE `{$this->sa_username_field}` = ? AND `{$this->sa_preference_field}` = ? AND `{$this->sa_value_field}` = ?;",
                    $this->sa_user,
                    'blacklist_from',
                    $email);

                // check address is not already whitelisted
                $sql_result = $db->query(
                                "SELECT `value` FROM `{$this->sa_table}` WHERE `{$this->sa_username_field}` = ? AND `{$this->sa_preference_field}` = ? AND `{$this->sa_value_field}` = ?;",
                                $this->sa_user,
                                'whitelist_from',
                                $email);

                if (!$db->fetch_array($sql_result)) {
                    $db->query(
                        "INSERT INTO `{$this->sa_table}` (`{$this->sa_username_field}`, `{$this->sa_preference_field}`, `{$this->sa_value_field}`) VALUES (?, ?, ?);",
                        $this->sa_user,
                        'whitelist_from',
                        $email);

                    if ($rcube->config->get('markasjunk2_debug')) {
                        rcube::write_log('markasjunk2', $this->sa_user . ' whitelist ' . $email);
                    }
                }
            }
        }
    }

    private function _do_salearn($uids, $spam)
    {
        $rcube = rcube::get_instance();
        $temp_dir = realpath($rcube->config->get('temp_dir'));
        $command = $rcube->config->get($spam ? 'markasjunk2_spam_cmd' : 'markasjunk2_ham_cmd');

        if (!$command)
            return;

        // backwards compatibility %xds removed in markasjunk2 v1.12
        $command = str_replace('%xds', '%h:x-dspam-signature', $command);

        $command = str_replace('%u', $_SESSION['username'], $command);
        $command = str_replace('%l', $rcube->user->get_username('local'), $command);
        $command = str_replace('%d', $rcube->user->get_username('domain'), $command);
        $command = str_replace('%m', $rcube->config->get('sauserprefs_db_dsnw'), $command);
        if (preg_match('/%i/', $command)) {
            $identity_arr = $rcube->user->get_identity();
            $command = str_replace('%i', $identity_arr['email'], $command);
        }

        foreach ($uids as $uid) {
            // reset command for next message
            $tmp_command = $command;

            // get DSPAM signature from header (if %xds macro is used)
            if (preg_match('/%xds/', $command)) {
                if (preg_match('/^X\-DSPAM\-Signature:\s+((\d+,)?([a-f\d]+))\s*$/im', $rcube->storage->get_raw_headers($uid), $dspam_signature))
                    $tmp_command = str_replace('%xds', $dspam_signature[1], $command);
                else
                    continue; // no DSPAM signature found in headers -> continue with next uid/message
            }

            if (strpos($tmp_command, '%s') !== false) {
                $message = new rcube_message($uid);
                $tmp_command = str_replace('%s', escapeshellarg($message->sender['mailto']), $tmp_command);
            }

            if (strpos($command, '%h') !== false) {
                $storage = $rcube->get_storage();
                $storage->check_connection();
                $storage->conn->select($src_mbox);

                preg_match_all('/%h:([\w-_]+)/', $tmp_command, $header_names, PREG_SET_ORDER);
                foreach ($header_names as $header) {
                    $val = null;
                    if ($msg = $storage->conn->fetchHeader($src_mbox, $uid, true, false, array($header[1]))) {
                        $val = $msg->{$header[1]} ?: $msg->others[$header[1]];
                    }

                    if (!empty($val)) {
                        $tmp_command = str_replace($header[0], escapeshellarg($val), $tmp_command);
                    }
                    else {
                        if ($rcube->config->get('markasjunk2_debug')) {
                            rcube::write_log('markasjunk2', 'header ' . $header[1] . ' not found in message ' . $src_mbox . '/' . $uid);
                        }

                        continue 2;
                    }
                }
            }

            if (strpos($command, '%f') !== false) {
                $tmpfname = tempnam($temp_dir, 'rcmSALearn');
                file_put_contents($tmpfname, $rcube->storage->get_raw_body($uid));
                $tmp_command = str_replace('%f', escapeshellarg($tmpfname), $tmp_command);
            }

            $output = shell_exec($tmp_command);

            if ($rcube->config->get('markasjunk2_debug')) {
                rcube::write_log('markasjunk2', $tmp_command);
                rcube::write_log('markasjunk2', $output);
            }

            if (strpos($command, '%f') !== false) {
                unlink($tmpfname);
            }
        }
    }
}
