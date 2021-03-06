import logging
import logging.handlers
import os


def disable_logger():
    logging.disable(logging.CRITICAL)


def init_logger():
    logger = logging.getLogger('evolution')
    logger.setLevel(logging.DEBUG)

    # File handler
    if not os.path.exists('./logs'):
        os.mkdir('./logs/')
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

# disable_logger()
