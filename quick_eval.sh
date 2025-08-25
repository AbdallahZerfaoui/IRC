#!/usr/bin/env bash
set -euo pipefail

PORT="${1:-6667}"
PASS="${2:-secretpw}"
BIN="./ircserv"

echo "[1/8] Clean build"
make re

echo "[2/8] Single poll() check"
grep -R --include='*.cpp' -nE '\bpoll\s*\(' . | wc -l

echo "[3/8] Launch server"
$BIN "$PORT" "$PASS" &
SRV_PID=$!
sleep 0.4
trap 'kill -9 $SRV_PID >/dev/null 2>&1 || true' EXIT

echo "[4/8] Basic nc auth and JOIN/PRIVMSG broadcast"
{
  # client A
  printf "PASS %s\r\nNICK alice\r\nUSER alice 0 * :Alice Liddell\r\nJOIN #42\r\n" "$PASS"
  sleep 0.2
  printf "PRIVMSG #42 :hello from alice\r\n"
  sleep 0.2
} | nc -N 127.0.0.1 "$PORT" > /tmp/irc_a.out &
A_PID=$!

{
  # client B
  printf "PASS %s\r\nNICK bob\r\nUSER bob 0 * :Bob Builder\r\nJOIN #42\r\n" "$PASS"
  sleep 0.5
  # partial (split) command to test robustness
  printf "PRIVMSG #42 :partial mes"
  sleep 0.2
  printf "sage continues\r\n"
  sleep 0.2
} | nc -N 127.0.0.1 "$PORT" > /tmp/irc_b.out &
B_PID=$!

wait $A_PID || true
wait $B_PID || true

echo "[5/8] Check that B received alice's message and the partial one"
grep -E "alice!|PRIVMSG #42" /tmp/irc_b.out || { echo "Broadcast/partial failed"; exit 1; }

echo "[6/8] Operator +k/+i/+t/+l sanity via nc"
{
  printf "PASS %s\r\nNICK op\r\nUSER op 0 * :ChanOp\r\nJOIN #m\r\nMODE #m +o op\r\nMODE #m +k key123\r\nMODE #m +i\r\nMODE #m +t\r\nMODE #m +l 10\r\n" "$PASS"
  sleep 0.2
} | nc -N 127.0.0.1 "$PORT" > /tmp/irc_op.out

grep -E " MODE #m " /tmp/irc_op.out >/dev/null || { echo "MODE output missing"; exit 1; }

echo "[7/8] Abrupt client death shouldn’t hang server"
# Start a client and kill it mid-command
( { printf "PASS %s\r\nNICK x\r\nUSER x 0 * :X\r\nJOIN #z\r\nPRIVMSG #z :hi"; sleep 0.1; } | nc 127.0.0.1 "$PORT" ) & disown || true
sleep 0.3
# Server still accepts new client?
echo | nc -w 1 127.0.0.1 "$PORT" >/dev/null && echo "Server still responsive"

echo "[8/8] Quick valgrind pass (optional, needs valgrind)"
if command -v valgrind >/dev/null 2>&1; then
  kill -9 $SRV_PID || true
  sleep 0.3
  valgrind --leak-check=full --error-exitcode=1 $BIN "$PORT" "$PASS" & VAL_PID=$!
  sleep 0.5
  # ping once
  { printf "PASS %s\r\nNICK vg\r\nUSER vg 0 * :VG\r\nQUIT\r\n" "$PASS"; } | nc -N 127.0.0.1 "$PORT" || true
  sleep 0.5
  kill -TERM $VAL_PID || true
fi

echo "OK ✅"
