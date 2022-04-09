import logging
import logging.handlers
import os


def init_logger():
    logger = logging.getLogger('evolution')
    logger.setLevel(logging.DEBUG)

    # File handler
    log_filename = './logs/evolution.log'
    should_roll_over = os.path.isfile(log_filename)
    log_handler = logging.handlers.RotatingFileHandler(
        log_filename, mode='w', backupCount=100, encoding='utf-8', delay=True)
    if should_roll_over:
        log_handler.doRollover()
    log_handler.setLevel(logging.DEBUG)

    logger.addHandler(log_handler)

    # write to stderr
    console_handler = logging.StreamHandler()
    console_handler.setLevel(logging.WARNING)
    logging.getLogger().addHandler(console_handler)

    # logging.disable()

# logging.basicConfig(filename="evolution.log",
#                     # filemode='a',
#                     format='%(asctime)s,%(msecs)d %(name)s %(levelname)s %(message)s',
#                     datefmt='%H:%M:%S',
#                     level=logging.DEBUG)
# logger = logging.getLogger('evolution')
