from sklearn.ensemble import RandomForestRegressor
from sklearn.preprocessing import StandardScaler
import matplotlib.pyplot as plt
from sklearn.svm import SVR
from sklearn.metrics import mean_squared_error, r2_score
from xgboost import XGBRegressor
from sklearn.model_selection import RandomizedSearchCV
from scipy.stats import uniform, randint
from sklearn.model_selection import GridSearchCV

def train_and_evaluate_rf(X_train, y_train, X_test, y_test, n_estimators=100, max_depth=None, random_state=42):
    # Train the model
    model = RandomForestRegressor(
        n_estimators=n_estimators,
        max_depth=max_depth,
        random_state=random_state
    )
    model.fit(X_train, y_train)
    
    # Predict
    y_pred = model.predict(X_test)
    
    # Evaluate
    mse = mean_squared_error(y_test, y_pred)
    r2 = r2_score(y_test, y_pred)
    
    print(f"Mean Squared Error (MSE): {mse:.4f}")
    print(f"R² Score: {r2:.4f}")
    
    # Plot True vs Predicted
    plt.figure(figsize=(12, 5))
    
    plt.subplot(1, 2, 1)
    plt.scatter(y_test, y_pred, alpha=0.7)
    plt.plot([y_test.min(), y_test.max()], [y_test.min(), y_test.max()], 'k--', lw=2)
    plt.xlabel('True Values')
    plt.ylabel('Predicted Values')
    plt.title('True vs Predicted')

    # Plot Residuals
    plt.subplot(1, 2, 2)
    residuals = y_test - y_pred
    plt.scatter(y_pred, residuals, alpha=0.7)
    plt.hlines(0, xmin=y_pred.min(), xmax=y_pred.max(), colors='r', linestyles='dashed')
    plt.xlabel('Predicted Values')
    plt.ylabel('Residuals')
    plt.title('Residuals Plot')
    
    plt.tight_layout()
    plt.show()
    
    return model

def rf_grid_search(X_train, y_train, X_test, y_test):
    param_grid = {
        'n_estimators': [100, 150, 200, 250, 300],
        'max_depth': [None, 10, 20],
        'max_features': ['sqrt', 'log2'],
        'bootstrap': [True, False]
    }

    rf_grid = GridSearchCV(estimator=RandomForestRegressor(random_state=42), param_grid=param_grid, cv=5, verbose=2, n_jobs=-1, scoring='r2')
    rf_grid.fit(X_train, y_train)
    
    # Predict
    y_pred = rf_grid.predict(X_test)
    
    # Evaluate
    mse = mean_squared_error(y_test, y_pred)
    r2 = r2_score(y_test, y_pred)

    print("Best Parameters:", rf_grid.best_params_)
    
    print(f"Mean Squared Error (MSE): {mse:.4f}")
    print(f"R² Score: {r2:.4f}")
    
    # Plot True vs Predicted
    plt.figure(figsize=(12, 5))
    
    plt.subplot(1, 2, 1)
    plt.scatter(y_test, y_pred, alpha=0.7)
    plt.plot([y_test.min(), y_test.max()], [y_test.min(), y_test.max()], 'k--', lw=2)
    plt.xlabel('True Values')
    plt.ylabel('Predicted Values')
    plt.title('True vs Predicted')

    # Plot Residuals
    plt.subplot(1, 2, 2)
    residuals = y_test - y_pred
    plt.scatter(y_pred, residuals, alpha=0.7)
    plt.hlines(0, xmin=y_pred.min(), xmax=y_pred.max(), colors='r', linestyles='dashed')
    plt.xlabel('Predicted Values')
    plt.ylabel('Residuals')
    plt.title('Residuals Plot')
    
    plt.tight_layout()
    plt.show()
    
    return rf_grid.best_estimator_, rf_grid.best_params_

def train_and_evaluate_svr(X_train, y_train, X_test, y_test, kernel='rbf', C=1.0, epsilon=0.1, gamma=0.1):
    # Scale the data (important for SVR!)
    scaler_X = StandardScaler()
    scaler_y = StandardScaler()
    
    X_train_scaled = scaler_X.fit_transform(X_train)
    X_test_scaled = scaler_X.transform(X_test)
    
    y_train_scaled = scaler_y.fit_transform(y_train.reshape(-1, 1)).ravel()
    
    # Train the SVR model
    model = SVR(kernel=kernel, C=C, epsilon=epsilon, gamma=gamma)
    model.fit(X_train_scaled, y_train_scaled)
    
    # Predict
    y_pred_scaled = model.predict(X_test_scaled)
    y_pred = scaler_y.inverse_transform(y_pred_scaled.reshape(-1, 1)).ravel()
    
    # Evaluate
    mse = mean_squared_error(y_test, y_pred)
    r2 = r2_score(y_test, y_pred)
    
    print(f"Mean Squared Error (MSE): {mse:.4f}")
    print(f"R² Score: {r2:.4f}")
    
    # Plot True vs Predicted
    plt.figure(figsize=(12, 5))
    
    plt.subplot(1, 2, 1)
    plt.scatter(y_test, y_pred, alpha=0.7)
    plt.plot([y_test.min(), y_test.max()], [y_test.min(), y_test.max()], 'k--', lw=2)
    plt.xlabel('True Values')
    plt.ylabel('Predicted Values')
    plt.title('True vs Predicted (SVR)')

    # Plot Residuals
    plt.subplot(1, 2, 2)
    residuals = y_test - y_pred
    plt.scatter(y_pred, residuals, alpha=0.7)
    plt.hlines(0, xmin=y_pred.min(), xmax=y_pred.max(), colors='r', linestyles='dashed')
    plt.xlabel('Predicted Values')
    plt.ylabel('Residuals')
    plt.title('Residuals Plot (SVR)')
    
    plt.tight_layout()
    plt.show()
    
    return model, scaler_X, scaler_y


def svr_grid_search(x_train, y_train, x_test, y_test):
    # Define the parameter grid
    param_grid = {
        'C': [0.1, 1, 10, 100],
        'epsilon': [0.01, 0.1, 0.2],
        'gamma': ['scale', 'auto', 0.01, 0.1, 1]
    }

    # Scale the data (important for SVR!)
    scaler_X = StandardScaler()
    scaler_y = StandardScaler()

    X_train_scaled = scaler_X.fit_transform(x_train)
    X_test_scaled = scaler_X.transform(x_test)

    y_train_scaled = scaler_y.fit_transform(y_train.reshape(-1, 1)).ravel()

    grid = GridSearchCV(SVR(kernel='rbf'), param_grid, cv=5, scoring='r2')
    grid.fit(X_train_scaled, y_train_scaled)

    print("Best parameters:", grid.best_params_)
    print("Best cross-validation score:", grid.best_score_)

    # Predict with the best model
    y_pred_scaled = grid.predict(X_test_scaled)
    y_pred = scaler_y.inverse_transform(y_pred_scaled.reshape(-1, 1)).ravel()
    mse = mean_squared_error(y_test, y_pred)
    r2 = r2_score(y_test, y_pred)
    print(f"Mean Squared Error (MSE): {mse:.4f}")
    print(f"R² Score: {r2:.4f}")
    # Plot True vs Predicted
    plt.figure(figsize=(12, 5))
    plt.subplot(1, 2, 1)
    plt.scatter(y_test, y_pred, alpha=0.7)
    plt.plot([y_test.min(), y_test.max()], [y_test.min(), y_test.max()], 'k--', lw=2)
    plt.xlabel('True Values')
    plt.ylabel('Predicted Values')
    plt.title('True vs Predicted (SVR)')
    # Plot Residuals
    plt.subplot(1, 2, 2)
    residuals = y_test - y_pred
    plt.scatter(y_pred, residuals, alpha=0.7)
    plt.hlines(0, xmin=y_pred.min(), xmax=y_pred.max(), colors='r', linestyles='dashed')
    plt.xlabel('Predicted Values')
    plt.ylabel('Residuals')
    plt.title('Residuals Plot (SVR)')
    plt.tight_layout()
    plt.show()

    return grid.best_estimator_, grid.best_params_, scaler_X, scaler_y


def train_and_evaluate_xgb(X_train, y_train, X_test, y_test, n_estimators=100, learning_rate=0.1, max_depth=6, random_state=42):
    # Scale the data (XGBoost works without scaling, but scaling sometimes still helps)
    scaler_X = StandardScaler()
    scaler_y = StandardScaler()
    
    X_train_scaled = scaler_X.fit_transform(X_train)
    X_test_scaled = scaler_X.transform(X_test)
    
    y_train_scaled = scaler_y.fit_transform(y_train.reshape(-1, 1)).ravel()
    
    # Train the XGBoost model
    model = XGBRegressor(
        n_estimators=n_estimators,
        learning_rate=learning_rate,
        max_depth=max_depth,
        random_state=random_state,
        objective='reg:squarederror'  # important for regression
    )
    model.fit(X_train_scaled, y_train_scaled)
    
    # Predict
    y_pred_scaled = model.predict(X_test_scaled)
    y_pred = scaler_y.inverse_transform(y_pred_scaled.reshape(-1, 1)).ravel()
    
    # Evaluate
    mse = mean_squared_error(y_test, y_pred)
    r2 = r2_score(y_test, y_pred)
    
    print(f"Mean Squared Error (MSE): {mse:.4f}")
    print(f"R² Score: {r2:.4f}")
    
    # Plot True vs Predicted
    plt.figure(figsize=(12, 5))
    
    plt.subplot(1, 2, 1)
    plt.scatter(y_test, y_pred, alpha=0.7)
    plt.plot([y_test.min(), y_test.max()], [y_test.min(), y_test.max()], 'k--', lw=2)
    plt.xlabel('True Values')
    plt.ylabel('Predicted Values')
    plt.title('True vs Predicted (XGBoost)')

    # Plot Residuals
    plt.subplot(1, 2, 2)
    residuals = y_test - y_pred
    plt.scatter(y_pred, residuals, alpha=0.7)
    plt.hlines(0, xmin=y_pred.min(), xmax=y_pred.max(), colors='r', linestyles='dashed')
    plt.xlabel('Predicted Values')
    plt.ylabel('Residuals')
    plt.title('Residuals Plot (XGBoost)')
    
    plt.tight_layout()
    plt.show()
    
    return model, scaler_X, scaler_y


def tune_xgb_hyperparameters_random(X_train, y_train, X_test, y_test, n_iter=20):
    # Scale the data
    scaler_X = StandardScaler()
    scaler_y = StandardScaler()

    X_train_scaled = scaler_X.fit_transform(X_train)
    X_test_scaled = scaler_X.transform(X_test)

    y_train_scaled = scaler_y.fit_transform(y_train.reshape(-1, 1)).ravel()

    # Define model
    model = XGBRegressor(objective='reg:squarederror', random_state=42)

    param_grid = {
        'n_estimators': [100, 150, 200, 250, 300],
        'max_depth': [3, 6, 10],
        'gamma': [0, 1, 5]
    }

    xgb_grid = GridSearchCV(estimator=model, param_grid=param_grid, cv=5, verbose=2, n_jobs=-1, scoring='r2')
    xgb_grid.fit(X_train_scaled, y_train_scaled)

    # Define parameter distributions (notice slight differences here)
    # param_distributions = {
    #     'n_estimators': randint(100, 500),
    #     'learning_rate': uniform(0.01, 0.3),
    #     'max_depth': randint(3, 10),
    #     'subsample': uniform(0.7, 0.3),
    #     'colsample_bytree': uniform(0.7, 0.3)
    # }

    # # Randomized Search
    # random_search = RandomizedSearchCV(
    #     estimator=model,
    #     param_distributions=param_distributions,
    #     n_iter=n_iter,  # number of random combinations to try
    #     scoring='r2',
    #     cv=3,
    #     verbose=1,
    #     n_jobs=-1,
    #     random_state=42
    # )

    # random_search.fit(X_train_scaled, y_train_scaled)

    print("Best Parameters Found:", xgb_grid.best_params_)
    print("Best R² Score Found:", xgb_grid.best_score_)

    # Predict with the best model
    y_pred_scaled = xgb_grid.predict(X_test_scaled)
    y_pred = scaler_y.inverse_transform(y_pred_scaled.reshape(-1, 1)).ravel()
    mse = mean_squared_error(y_test, y_pred)
    r2 = r2_score(y_test, y_pred)
    print(f"Mean Squared Error (MSE): {mse:.4f}")
    print(f"R² Score: {r2:.4f}")

    # Plot True vs Predicted
    plt.figure(figsize=(12, 5))
    plt.subplot(1, 2, 1)
    plt.scatter(y_test, y_pred, alpha=0.7)
    plt.plot([y_test.min(), y_test.max()], [y_test.min(), y_test.max()], 'k--', lw=2)
    plt.xlabel('True Values')
    plt.ylabel('Predicted Values')
    plt.title('True vs Predicted (XGBOOST)')

    # Plot Residuals
    plt.subplot(1, 2, 2)
    residuals = y_test - y_pred
    plt.scatter(y_pred, residuals, alpha=0.7)
    plt.hlines(0, xmin=y_pred.min(), xmax=y_pred.max(), colors='r', linestyles='dashed')
    plt.xlabel('Predicted Values')
    plt.ylabel('Residuals')
    plt.title('Residuals Plot (XGBOOST)')
    plt.tight_layout()
    plt.show()

    return xgb_grid.best_estimator_, xgb_grid.best_params_, scaler_X, scaler_y
