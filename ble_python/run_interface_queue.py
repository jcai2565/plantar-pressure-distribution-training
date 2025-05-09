# run_interface_queue.py

import asyncio
import multiprocessing
import time
from ble import get_ble_controller
from cmd_types import CMD
from foot_gui.foot_pressure_pygame import run_pressure_gui
from utils import setup_logging

LOG = setup_logging("interface_queue.log")
RX_UUID_KEY = 'RX_STRING'

def gui_process(queue):
    latest = [None] * 6

    def get_latest_pressure():
        nonlocal latest
        while not queue.empty():
            latest = queue.get()
        return latest

    run_pressure_gui(get_latest_pressure)

async def main():
    queue = multiprocessing.Queue()
    gui_worker = multiprocessing.Process(target=gui_process, args=(queue,))
    gui_worker.start()

    ble = get_ble_controller()
    await ble.device._connect()
    LOG.info("BLE connected successfully.")

    def pressure_ble_handler(_, data_bytes):
        try:
            s = data_bytes.decode().strip()
            values = [x.split(":")[1] for x in s.split("|")]
            parsed = [None if v == "None" else float(v) for v in values]
            queue.put(parsed)
        except Exception as e:
            LOG.warning(f"BLE parse error: {e}")

    rx_uuid = ble.uuid[RX_UUID_KEY]
    await ble.device._start_notify(rx_uuid, pressure_ble_handler)
    ble.send_command(CMD.START_STREAM, "")
    LOG.info("BLE streaming started.")

    try:
        while gui_worker.is_alive():
            await asyncio.sleep(0.1)
    finally:
        LOG.info("Stopping BLE...")
        await ble.device._stop_notify(rx_uuid)
        ble.send_command(CMD.STOP_STREAM, "")
        gui_worker.join()
        LOG.info("Exited cleanly.")

if __name__ == "__main__":
    import nest_asyncio
    import sys

    try:
        nest_asyncio.apply()
        asyncio.run(main())
    except KeyboardInterrupt:
        LOG.info("Program interrupted by user (Ctrl+C)")
        sys.exit(0)
    except Exception as e:
        LOG.error(f"Unhandled exception: {e}")
        sys.exit(1)
