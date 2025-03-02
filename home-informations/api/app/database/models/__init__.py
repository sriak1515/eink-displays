from .eink.display import Display, DisplayPublic, DisplayPublicWithUpdates
from .eink.update import Update, UpdatePublic, UpdatePublicWithDisplay

DisplayPublicWithUpdates.model_rebuild()
UpdatePublicWithDisplay.model_rebuild()